#include <iostream>

#include <list>
#include <set>
#include <map>
#include <future>
#include <functional>
#include <thread>

#include <assert.h>

// atomic/atomic_base needs to be finished
// local_atomic_future needs to be implemented
// RAII locking needs to be added in (should synergize with atomic/atomic_base)
// * Probably need a locking base class too.
// transaction should be in a shared_ptr; local_ objects should hold references
// each async {} block needs to hold a clone()'d copy of any atomic_base variables it uses.

// inside of an async {} block, they can only be read

// lock_list will need to be sorted.

// IO.then([] { /* write my 100GB of plaintext doubles to file */ });

// if (/* some atomic condition */) then /* start some async thing to make a bitcoin */

struct atomic_base
{
    virtual ~atomic_base() {}

    virtual atomic_base* clone() const = 0;

    virtual void write(atomic_base const&) = 0;

    virtual std::unique_lock<std::mutex> lock() const = 0;

    virtual bool operator==(atomic_base const&) = 0;
};

struct transaction;

template <typename T>
struct atomic : atomic_base
{
    struct local_atomic
    {
      private:
        transaction* trans_;
        atomic_base* var_;

      public:
        local_atomic(transaction* trans, atomic_base* var)
          : trans_(trans)
          , var_(var)
        {}

        operator T const& ();

        local_atomic& operator=(atomic_base const& rhs);

        local_atomic& operator=(T const& rhs);
    };

  private:
    T data_;
    mutable std::mutex mtx_;

  public:
    atomic() : data_() {}

    atomic(T const& t) : data_(t) {}

    atomic(T&& t) : data_(t) {}

    ~atomic() { }

    // Locks.
    atomic_base* clone() const
    {
        auto l = lock();
        return new atomic(data_);
    }

    // Doesn't lock.
    T const& read() const
    {
        return data_;
    }

    // Doesn't lock.
    void write(T const& rhs)
    {
        data_ = rhs;
    }

    // Doesn't lock.
    void write(atomic_base const& rhs)
    {
        data_ = dynamic_cast<atomic const*>(&rhs)->read();
    }

    std::unique_lock<std::mutex> lock() const
    {
        return std::unique_lock<std::mutex>(mtx_); 
    }

    bool operator==(atomic_base const& rhs)
    {
        return data_ == dynamic_cast<atomic const*>(&rhs)->read(); 
    }

    local_atomic get_local(transaction& trans)
    {
        return local_atomic(&trans, this); 
    }
};

struct transaction
{
    std::list<
        std::pair<
            // The atomic variable we're reading from
            atomic_base* 
            // The value we read from the variable
          , std::shared_ptr<atomic_base>
        >
    > read_list;

    std::set<
        atomic_base* // The atomic variable we're writing to
    > write_set;

    std::list<
        std::pair<
            std::future<void>*    // The future we're writing to
                                  // (if NULL, fire-and-forget semantics)
          , std::function<void()> // The async action to execute
        >
    > async_list;

    std::map<
        atomic_base* // The atomic variable we're reading from
      , std::shared_ptr<atomic_base> // Current value of the variable
    > variables;

    void clear()
    {
        read_list.clear();
        write_set.clear();
        async_list.clear();
        variables.clear();
    }

    // This should possibly be called RAII-style from the destructor. 
    bool commit_transaction()
    {
        // Algorithm:
        //
        // 1.) Obtain exclusive access to all the variables. 
        // 2.) Compare our recorded reads against the current values (and fail if needed).
        // 3.) Perform writes, reading from our internal map.
        // 4.) Perform async operations.
        // 5.) Release exclusive access.

        // 1.) Obtain exclusive access to all the variables. 
        std::list<std::unique_lock<std::mutex> > locks;

        for ( std::pair<atomic_base*, std::shared_ptr<atomic_base> > const& var
            : variables)
        {
            assert(var.first != NULL);

            locks.push_back((*var.first).lock());
        }
        
        // 2.) Compare our recorded reads against the current values (and fail if needed).
        for ( std::pair<atomic_base*, std::shared_ptr<atomic_base> > const& var
            : read_list)
        {
            assert(var.first != NULL);

            if (!((*var.first) == (*var.second))) // Perform read operation.
            {
                clear();

                // Transaction fails; exclusive access is released by RAII.
                // Note that this leaves any futures used in a default-constructed
                // state - perhaps we should put an exception (transaction_failed)
                // into them instead. 
                return false;
            }
        }

        // 3.) Perform writes, reading from our internal map.
        for (atomic_base* var : write_set)
        {
            assert(var != NULL);

            auto it = variables.find(var);

            assert(it != variables.end());

            (*var).write((*(*it).second)); // Perform write operation.
        }

        // 4.) Perform async operations.
        for ( std::pair<std::future<void>*, std::function<void()> > const& op
            : async_list)
        {
            // If the future is void, use fire-and-forget semantics.
            if (op.first == NULL)
                // For HPX version, just use hpx::apply.
                std::async(op.second);
            else
                (*op.first) = std::async(op.second);
        }

        // 5.) Release exclusive access.
        return true; 
    }

    atomic_base const& read(atomic_base* var)
    {
        // We have two cases here:
        // * The variable has not been read or written in this transaction, and is
        //   not present in the internal state.
        // * The variable has been read or written previously in this transaction,
        //   and is present in the internal state.

        assert(var != NULL);

        // We will attempt an insertion with a placeholder value, to avoid a
        // speculative read and performing the map lookup twice (find then insert)
        // for first-time entries.
        std::pair<atomic_base*, std::shared_ptr<atomic_base> > entry(var, 0);

        // This auto will be a std::pair<iterator, bool>.
        auto result = variables.insert(entry);

        if (result.second == true)
        {
            // Insertion succeeded; this is the first read of the variable.
            (*result.first).second.reset((*var).clone()); // Perform read. 

            // Record the read operation.
            read_list.push_back(*result.first);

            return (*(*result.first).second);
        }
        else
            // Insertion failed; the variable is in the internal state.
            return (*(*result.first).second);
    }

    void write(atomic_base* var, atomic_base const& value)
    {
        // We have two cases here:
        // * The variable has not been read or written in this transaction, and is
        //   not present in the internal state.
        // * The variable has been read or written previously in this transaction,
        //   and is present in the internal state.

        assert(var != NULL);

        // We will attempt an insertion, to avoid performing the map lookup twice
        // (find then insert) for first-time entries.
        std::pair<atomic_base*, std::shared_ptr<atomic_base> >
            entry(var, std::shared_ptr<atomic_base>(value.clone()));

        // This auto will be a std::pair<iterator, bool>.
        auto result = variables.insert(entry);

        if (result.second == true)
        {
            // Insertion succeeded; this is the first read of the variable.
            
            // Make sure the variable is in the write set.
            write_set.insert(var); 
        }
        else
        { 
            // Insertion failed; the variable is in the internal state.
            (*result.first).second = entry.second; // Perform INTERNAL write.
            
            // Make sure the variable is in the write set.
            write_set.insert(var); 
        }
    }

    // If fut is NULL, then fire-and-forget semantics are used.
    void async(std::future<void>* fut, std::function<void()> F)
    {
        std::pair<std::future<void>*, std::function<void()> > entry(fut, F);
        async_list.push_back(entry);
    }
};

template <typename T>
atomic<T>::local_atomic::operator T const& () 
{
    return dynamic_cast<atomic const*>(&trans_->read(var_))->read();
}

template <typename T>
typename atomic<T>::local_atomic&
atomic<T>::local_atomic::operator=(atomic_base const& rhs)
{
    trans_->write(var_, rhs);
    return *this;
}

template <typename T>
typename atomic<T>::local_atomic&
atomic<T>::local_atomic::operator=(T const& rhs)
{
    atomic tmp(rhs);
    trans_->write(var_, tmp);
    return *this;
}

int main()
{
    {
        atomic<int> A(4);
        atomic<int> B(1);

        // atomic { A = A*A - B; }
        transaction t;
        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_ - B_;
        } while (!t.commit_transaction());

        std::cout << "A = " << A.read() << "\n"
                  << "B = " << B.read() << "\n"; 
    }

    {
        atomic<int> A(4);
        atomic<int> B(1);
        std::future<void> IO;

        // future<void> IO;
        // atomic {
        //     A = A*A;
        //     IO = async { std::cout << A << "\n"; }
        //     A = A - B;
        // }
        transaction t;
        do {
            auto A_ = A.get_local(t);
            auto B_ = B.get_local(t);

            A_ = A_*A_;

            t.async(&IO,
                [local_A = int(A_)] () { std::cout << local_A << "\n"; }
            );

            A_ = A_ - B_;
        } while (!t.commit_transaction());

        std::cout << "A = " << A.read() << "\n"
                  << "B = " << B.read() << "\n"; 

        IO.get();
    }

    {
        atomic<int> A(4);

        // atomic { A = A*A; }
        bool fail = true;
        int attempt_count = 0;
        transaction t;
        do {
            ++attempt_count;

            auto A_ = A.get_local(t);

            int tmp = A_*A_;

            if (fail)
            {
                A.write(3);
                fail = false;
            }

            A_ = tmp; 
        } while (!t.commit_transaction());

        std::cout << "A = " << A.read() << "\n"
                  << "Attempts: " << attempt_count << "\n"; 
    }
/*

    std::cout << "A = " << A << "\n"
              << "B = " << B << "\n"; 

    IO.get();

    A = 4;

    // atomic {
    //     A = A*A;
    // }
    bool fail_first = false;
    int attempt_count = 0;
    transaction t2;
    do {
        ++attempt_count;

        local_atomic A_(t2, &A);

        int tmp = A_*A_;

        if (!fail_first)
        {
            A = 3;
            fail_first = true;
        }

        // A = A*A
        A_ = tmp;
    } while (!t2.commit_transaction());

    std::cout << "A = " << A << "\n"
              << "Attempts: " << attempt_count << "\n"; 
*/
}
