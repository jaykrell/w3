// A WebAssembly codebase by Jay Krell
//
// https://webassembly.github.io/spec/core/binary/index.html
// https://webassembly.github.io/spec/core/_download/WebAssembly.pdf

#pragma once

#include "w3StackBase.h"
#include "w3StackValue.h"
#include "w3Tag.h"
#include "w3Value.h"

// work in progress
struct Stack : private StackBase
{
    Stack () { }

    // old compilers lack using.
    typedef StackBase base;
    typedef base::iterator iterator;
    void pop () { base::pop (); }
    StackValue& top () { return base::top (); }
    StackValue& back () { return base::back (); }
    StackValue& front () { return base::front (); }
    iterator begin () { return base::begin (); }
    iterator end () { return base::end (); }
    bool empty () const { return base::empty (); }
    void resize (size_t newsize) { base::resize (newsize); }
    size_t size () { return base::size (); }
    StackValue& operator [ ] (size_t index) { return base::operator [ ] (index); }

    void reserve (size_t n)
    {
        // TODO
    }

    // While ultimately a stack of values, labels, and frames, values dominate.

    Tag& tag (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.tag;
    }

    Tag& tag ()
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        return t.value.tag;
    }

    Value& value ()
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        return t.value.value;
    }

    Value& value (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        AssertFormat (t.value.tag == tag, ("%X %X", t.value.tag, tag));
        return t.value.value;
    }

    void pop_label ()
    {
        if (size () < 1 || top ().tag != Tag_Label)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == Tag_Label, ("%X %X", top ().tag, Tag_Label));
        pop ();
    }

    void pop_value ()
    {
        AssertTopIsValue ();
        //int t = tag ();
        pop ();
        //printf ("pop_value tag:%s depth:%" FORMAT_SIZE "X\n", TagToString (t), size ());
    }

    void push_value (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Value, ("%X %X", value.tag, Tag_Value));
        push (value);
        //printf ("push_value tag:%s value:%X depth:%" FORMAT_SIZE "X\n", TagToString (value.value.tag), value.value.value.i32, size ());
    }

    void push_label (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Label, ("%X %X", value.tag, Tag_Label));
        push (value);
        //printf ("push_label depth:%" FORMAT_SIZE "X\n", size ());
    }

    void push_frame (const StackValue& value)
    {
        AssertFormat (value.tag == Tag_Frame, ("%X %X", value.tag, Tag_Frame));
        push (value);
        //printf ("push_frame depth:%" FORMAT_SIZE "X\n", size ());
    }

    // type specific pushers

    void push_i32 (int32_t i)
    {
        StackValue value (Tag_i32);
        value.value.value.i32 = i;
        push_value (value);
    }

    void push_i64 (int64_t i)
    {
        StackValue value (Tag_i64);
        value.value.value.i64 = i;
        push_value (value);
    }

    void push_u32 (uint32_t i)
    {
        push_i32 ((int32_t)i);
    }

    void push_u64 (uint64_t i)
    {
        push_i64 ((int64_t)i);
    }

    void push_f32 (float i)
    {
        StackValue value (Tag_f32);
        value.value.value.f32 = i;
        push_value (value);
    }

    void push_f64 (double i)
    {
        StackValue value (Tag_f64);
        value.value.value.f64 = i;
        push_value (value);
    }

    void push_bool (bool b)
    {
        push_i32 (b);
    }

    // accessors, check tag, return ref

    int32_t& i32 ()
    {
        return value (Tag_i32).i32;
    }

    int64_t& i64 ()
    {
        return value (Tag_i64).i64;
    }

    uint32_t& u32 ()
    {
        return value (Tag_i32).u32;
    }

    uint64_t& u64 ()
    {
        return value (Tag_i64).u64;
    }

    float& f32 ()
    {
        return value (Tag_f32).f32;
    }

    double& f64 ()
    {
        return value (Tag_f64).f64;
    }

    void DumpStack (PCSTR prefix)
    {
        const size_t n = size ();
        printf ("stack@%s: %" FORMAT_SIZE "X ", prefix, n);
        for (size_t i = 0; i != n; ++i)
        {
            printf ("%s:", TagToString (begin () [(ssize_t)i].tag));
            switch (begin () [(ssize_t)i].tag)
            {
            case Tag_Label:
                break;
            case Tag_Frame:
                break;
            case Tag_Value:
                printf ("%s", TagToString (begin () [(ssize_t)i].value.tag));
                break;
            default:; //todo
            }
            printf (" ");
        }
        printf ("\n");
    }

    void AssertTopIsValue ()
    {
        if (size () < 1 || top ().tag != Tag_Value)
            DumpStack ("AssertTopIsValue");
        AssertFormat (size () >= 1, ("%" FORMAT_SIZE "X", size ()));
        AssertFormat (top ().tag == Tag_Value, ("%X %X", top ().tag, Tag_Value));
    }

    // setter, changes tag, returns ref

    Value& set (Tag tag)
    {
        AssertTopIsValue ();
        StackValue& t = top ();
        TaggedValue& v = t.value;
        v.tag = tag;
        return v.value;
    }

    // type-specific setters

    void set_i32 (int32_t a)
    {
        set (Tag_i32).i32 = a;
    }

    void set_u32 (uint32_t a)
    {
        set (Tag_i32).u32 = a;
    }

    void set_bool (bool a)
    {
        set_i32 (a);
    }

    void set_i64 (int64_t a)
    {
        set (Tag_i64).i64 = a;
    }

    void set_u64 (uint64_t a)
    {
        set (Tag_i64).u64 = a;
    }

    void set_f32 (float a)
    {
        set (Tag_f32).f32 = a;
    }

    void set_f64 (double a)
    {
        set (Tag_f64).f64 = a;
    }

    // type specific poppers

    int32_t pop_i32 ()
    {
        int32_t a = i32 ();
        pop_value ();
        return a;
    }

    uint32_t pop_u32 ()
    {
        uint32_t a = u32 ();
        pop_value ();
        return a;
    }

    int64_t pop_i64 ()
    {
        int64_t a = i64 ();
        pop_value ();
        return a;
    }

    uint64_t pop_u64 ()
    {
        uint64_t a = u64 ();
        pop_value ();
        return a;
    }

    float pop_f32 ()
    {
        float a = f32 ();
        pop_value ();
        return a;
    }

    double pop_f64 ()
    {
        double a = f64 ();
        pop_value ();
        return a;
    }
};
