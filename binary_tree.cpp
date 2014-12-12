////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Bryce Adelstein-Lelbach
//  Copyright (c) 2014 Steve Brandt 
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <iostream>

#include <cstdint>

#include "astm.hpp"

#include "astm_new.hpp"

#ifdef ASTM_HPX
    #include <hpx/hpx_main.hpp>
#endif

using namespace astm;

template <typename Key, typename Value>
struct binary_tree
{
    struct node
    {
        shared_var<Key> key;
        Value value;
        shared_var<node*> left;
        shared_var<node*> right;

        node()
          : key(), value(), left(), right()
        {}

        node(Key k, Value const& v, node* l, node* r)
          : key(k), value(v), left(l), right(r)
        {} 

        node(Key k, Value const& v, shared_var<node*> l, shared_var<node*> r)
          : key(k), value(v), left(l), right(r)
        {} 

        node(Key k, Value const& v)
          : key(k), value(v), left(), right()
        {} 

/*
        bool operator==(node const& rhs) const
        {
            return key == rhs.key; 
        }
*/
    };

    binary_tree()
      : root()
    {}

    ~binary_tree()
    {
//        destroy(root);
    }

    // Initiates transaction.
    void insert(Key key, Value const& value)
    {
        transaction t;
        do {
            auto root_ = root.get_local(t);

            if (root_.get() != 0)
                insert(key, value, root, t);
            else
                root_ = new node(key, value);
        } while (!t.commit_transaction());
    }

    // Initiates transaction.
    node* search(Key key)
    {
        node* n;

        transaction t;
        do {
            n = search(key, root, t);
        } while (!t.commit_transaction());

        return n;
    }

  private:
/*
    // Initiates transaction.
    void destroy(node* leaf)
    {
        transaction t;
        do {
            destroy(leaf, t);
        } while (!t.commit_transaction());
    }

    void destroy(node* leaf, transaction& t)
    {
        if (leaf != 0)
        {
            destroy(leaf->left, t);
            destroy(leaf->right, t);
            delete leaf;
        }
    }
*/

    void insert(Key key, Value const& value, shared_var<node*>& leaf, transaction& t)
    {
        auto leaf_ = leaf.get_local(t);

        if (key < leaf_.get()->key.get_local(t).get())
        {
            if (leaf_.get()->left.get_local(t).get() != 0)
                insert(key, value, leaf_.get()->left, t);
            else
                // FIXME: Unnecessary deallocation here.
                leaf_.get()->left.get_local(t) = new node(key, value); 
        }

        else if (key >= leaf_.get()->key.get_local(t).get())
        {
            if (leaf_.get()->right.get_local(t).get() != 0)
                insert(key, value, leaf_.get()->right, t);
            else
                // Allocation required here.
                leaf_.get()->right.get_local(t) = new node(key, value);
        }
    }

    node* search(Key key, shared_var<node*>& leaf, transaction& t)
    {
        auto leaf_ = leaf.get_local(t);

        if (leaf_.get() != 0)
        {
            if (key == leaf_.get()->key.get_local(t).get())
                return leaf_.get();
            if (key < leaf_.get()->key.get_local(t).get())
                return search(key, leaf_.get()->left, t);
            else
                return search(key, leaf_.get()->right, t);
        }
        else
        {
            return 0; 
        }
    }

    shared_var<node*> root;
};

int main()
{
    {
        binary_tree<int, std::string> tree;    

        tree.insert(1, "hello");

        auto n = tree.search(1);

        std::cout << n->value << "\n"; 
    }

    return 0;
}

