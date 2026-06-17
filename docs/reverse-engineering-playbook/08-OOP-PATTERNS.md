# 08 - Object-Oriented Patterns

## Vtables

A vtable is an array of function pointers stored in `.rdata`.

```cpp
// Constructor assigns vtable first
this->vtable = &KnownVtable;

// Slot N = vtable + N*4 (32-bit) or N*8 (64-bit)
```

Use vtables to map:
- Destructor slot 0
- Virtual methods 1..N
- Multiple inheritance may have multiple vtables

## Constructors / Destructors

```cpp
// In constructor:
this->vtable = &LiveVtable;   // most-derived
SubObject_ctor(this + N);

// In destructor:
this->vtable = &BaseVtable;    // base
SubObject_dtor(this + N);
operator_delete(this);
```

## Multiple Inheritance

A class with multiple bases has multiple vtable pointers, often at `+0`, `+N`.
Watch for destructors that restore a different vtable than the constructor sets.
