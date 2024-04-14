
#ifndef UTILITIES_H
#define UTILITIES_H


// Index sequence implementation
template <int... Is>
struct index_sequence
{
};

template <int N, int... Is>
struct make_index_sequence : make_index_sequence<N - 1, N - 1, Is...>
{
};

template <int... Is>
struct make_index_sequence<0, Is...> : index_sequence<Is...>
{
};

/*-----------------------------------------------------------------------------------------------------------*/

// Tuple-like structure using template recursion
template <typename... Args>
struct Tuple
{
};

// Forward declare the empty Tuple specialization
template <>
struct Tuple<>
{
    Tuple() {}
};

template <typename T, typename... Args>
struct Tuple<T, Args...> : public Tuple<Args...>
{
    T head;
    Tuple<Args...> tail;

    Tuple(T h, Args... args) : head(h), tail(args...) {}
    Tuple() : head(T()), tail() {}

    template <int N>
    auto get() -> typename std::conditional<N == 0, T &, decltype(tail.template get<N - 1>())>::type
    {
        if constexpr (N == 0)
            return head;
        else
            return tail.template get<N - 1>();
    }
};

template <typename T>
struct Tuple<T> : public Tuple<>
{
    T head;

    Tuple(T h) : head(h) {}
    Tuple() : head(T()) {}

    template <int N>
    auto get() -> typename std::conditional<N == 0, T &, void>::type
    {
        static_assert(N == 0, "Index out of bounds in Tuple::get");
        return head;
    }
};


/*---------------------------------------------------------------------------------------------------------------------------------*/



//Queue implementation.
template <typename T>
class Queue
{
private:
    struct Node
    {
        T *task;
        Node *next;
    };
    Node *head;
    Node *tail;

public:
    Queue() : head(nullptr), tail(nullptr) {}

    void push(T *task)
    {
        Node *newNode = new Node{task, nullptr};
        if (!head)
        {
            head = newNode;
            tail = newNode;
        }
        else
        {
            tail->next = newNode;
            tail = newNode;
        }
    }

    bool empty()
    {
        return head == nullptr;
    }

    T *front()
    {
        return head ? head->task : nullptr;
    }

    void pop()
    {
        if (!head)
            return;
        Node *temp = head;
        head = head->next;
        delete temp;
    }
};

#endif // UTILITIES_H