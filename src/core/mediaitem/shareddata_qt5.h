#ifndef QSHAREDDATA_H
#define QSHAREDDATA_H

#include <QtCore/qglobal.h>
#include <QtCore/qatomic.h>


template <class T> class QExplicitlySharedDataPointer
{
public:
    typedef T Type;
    typedef T *pointer;

    inline T &operator*() const { return *d; }
    inline T *operator->() { return d; }
    inline T *operator->() const { return d; }
    inline T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline void detach() { if (d && d->ref.load() != 1) detach_helper(); }

    inline void reset()
    {
        if(d && !d->ref.deref())
            delete d;

        d = 0;
    }

    inline operator bool () const { return d != 0; }

    inline bool operator==(const QExplicitlySharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const QExplicitlySharedDataPointer<T> &other) const { return d != other.d; }
    inline bool operator==(const T *ptr) const { return d == ptr; }
    inline bool operator!=(const T *ptr) const { return d != ptr; }

    inline QExplicitlySharedDataPointer() { d = 0; }
    inline ~QExplicitlySharedDataPointer() { if (d && !d->ref.deref()) delete d; }

    explicit QExplicitlySharedDataPointer(T *data);
    inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<T> &o) : d(o.d) { if (d) d->ref.ref(); }

    template<class X>
    inline QExplicitlySharedDataPointer(const QExplicitlySharedDataPointer<X> &o) : d(static_cast<T *>(o.data()))
    {
        if(d)
            d->ref.ref();
    }

    inline QExplicitlySharedDataPointer<T> & operator=(const QExplicitlySharedDataPointer<T> &o) {
        if (o.d != d) {
            if (o.d)
                o.d->ref.ref();
            T *old = d;
            d = o.d;
            if (old && !old->ref.deref())
                delete old;
        }
        return *this;
    }
    inline QExplicitlySharedDataPointer &operator=(T *o) {
        if (o != d) {
            if (o)
                o->ref.ref();
            T *old = d;
            d = o;
            if (old && !old->ref.deref())
                delete old;
        }
        return *this;
    }
#ifdef Q_COMPILER_RVALUE_REFS
    inline QExplicitlySharedDataPointer(QExplicitlySharedDataPointer &&o) : d(o.d) { o.d = 0; }
    inline QExplicitlySharedDataPointer<T> &operator=(QExplicitlySharedDataPointer<T> &&other)
    { qSwap(d, other.d); return *this; }
#endif

    inline bool operator!() const { return !d; }

    inline void swap(QExplicitlySharedDataPointer &other)
    { qSwap(d, other.d); }

protected:
    T *clone();

private:
    void detach_helper();

    T *d;
};

template <class T>
Q_INLINE_TEMPLATE QSharedDataPointer<T>::QSharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

template <class T>
Q_INLINE_TEMPLATE T *QSharedDataPointer<T>::clone()
{
    return new T(*d);
}

template <class T>
Q_OUTOFLINE_TEMPLATE void QSharedDataPointer<T>::detach_helper()
{
    T *x = clone();
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

template <class T>
Q_INLINE_TEMPLATE T *QExplicitlySharedDataPointer<T>::clone()
{
    return new T(*d);
}

template <class T>
Q_OUTOFLINE_TEMPLATE void QExplicitlySharedDataPointer<T>::detach_helper()
{
    T *x = clone();
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

template <class T>
Q_INLINE_TEMPLATE QExplicitlySharedDataPointer<T>::QExplicitlySharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

template <class T>
Q_INLINE_TEMPLATE void qSwap(QSharedDataPointer<T> &p1, QSharedDataPointer<T> &p2)
{ p1.swap(p2); }

template <class T>
Q_INLINE_TEMPLATE void qSwap(QExplicitlySharedDataPointer<T> &p1, QExplicitlySharedDataPointer<T> &p2)
{ p1.swap(p2); }

QT_END_NAMESPACE
namespace std {
    template <class T>
    Q_INLINE_TEMPLATE void swap(QT_PREPEND_NAMESPACE(QSharedDataPointer)<T> &p1, QT_PREPEND_NAMESPACE(QSharedDataPointer)<T> &p2)
    { p1.swap(p2); }

    template <class T>
    Q_INLINE_TEMPLATE void swap(QT_PREPEND_NAMESPACE(QExplicitlySharedDataPointer)<T> &p1, QT_PREPEND_NAMESPACE(QExplicitlySharedDataPointer)<T> &p2)
    { p1.swap(p2); }
}
QT_BEGIN_NAMESPACE

template<typename T> Q_DECLARE_TYPEINFO_BODY(QSharedDataPointer<T>, Q_MOVABLE_TYPE);
template<typename T> Q_DECLARE_TYPEINFO_BODY(QExplicitlySharedDataPointer<T>, Q_MOVABLE_TYPE);

QT_END_NAMESPACE

#endif // QSHAREDDATA_H
