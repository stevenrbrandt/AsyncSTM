#include <hpx/hpx_main.hpp>

#include <iostream>

#include "async_stm.hpp"

using namespace astm;

template <typename Key, typename Value>
struct tree_node
{
    atomic<Key*> key;
    Value* val;
    atomic<tree_node*> lt; // lt->key < key
    atomic<tree_node*> gt; // key < lt->key

    void insert(Key k, Value const& v)
    {
        if (k == *key)
            // insertion fails

        if (k < *key) 
        {
            if (lt)
            {
                if (k == lt->key)
                {
                    // insertion fails, would overwrite
                }
                else if (k < lt->key)
                {
                    lt->insert(k, v);
                }
                else
                {
                    tree_node* new_node = new tree_node(k, v, lt, this);
                    lt = new_node;
                }
            }
            else
            {
                tree_node* new_node = new tree_node(k, v, NULL, this);
                lt = new_node;
            }
        }

        // else // similar
    }
};

 
