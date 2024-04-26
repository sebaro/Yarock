/****************************************************************************************
*  YAROCK                                                                               *
*  Copyright (c) 2010-2018 Sebastien amardeilh <sebastien.amardeilh+yarock@gmail.com>   *
*                                                                                       *
*  This program is free software; you can redistribute it and/or modify it under        *
*  the terms of the GNU General Public License as published by the Free Software        *
*  Foundation; either version 2 of the License, or (at your option) any later           *
*  version.                                                                             *
*                                                                                       *
*  This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
*  PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
*                                                                                       *
*  You should have received a copy of the GNU General Public License along with         *
*  this program.  If not, see <http://www.gnu.org/licenses/>.                           *
*****************************************************************************************/

#ifndef _SHARED_DATA_H_
#define _SHARED_DATA_H_

#include <QSharedData>

/*
  class ExplicitlySharedDataPointer
  -> copy of QExplicitlySharedDataPointer from Qt source
  -> add static cast without count reference increment
*/

template <class T> class ExplicitlySharedDataPointer
{
public:
    typedef T Type;
    typedef T *pointer;

    inline T &operator*() const { return *d; }
    inline T *operator->() { return d; }
    inline T *operator->() const { return d; }
    inline T *data() const { return d; }
    inline const T *constData() const { return d; }

    inline void detach() { if (d && d->ref.loadRelaxed() != 1) detach_helper(); }

    inline void reset()
    {
        if(d && !d->ref.deref())
            delete d;

        d = 0;
    }

    inline operator bool () const { return d != 0; }

    inline bool operator==(const ExplicitlySharedDataPointer<T> &other) const { return d == other.d; }
    inline bool operator!=(const ExplicitlySharedDataPointer<T> &other) const { return d != other.d; }
    inline bool operator==(const T *ptr) const { return d == ptr; }
    inline bool operator!=(const T *ptr) const { return d != ptr; }

    inline ExplicitlySharedDataPointer() { d = 0; }
    inline ~ExplicitlySharedDataPointer() { if (d && !d->ref.deref()) delete d; }

    explicit ExplicitlySharedDataPointer(T *data);
    inline ExplicitlySharedDataPointer(const ExplicitlySharedDataPointer<T> &o) : d(o.d) { if (d) d->ref.ref(); }

    template<class X>
    inline ExplicitlySharedDataPointer(const ExplicitlySharedDataPointer<X> &o) : d(static_cast<T *>(o.data()))
    {
        if(d)
          d->ref.ref();
    }

    inline ExplicitlySharedDataPointer<T> & operator=(const ExplicitlySharedDataPointer<T> &o)
    {
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

    inline ExplicitlySharedDataPointer &operator=(T *o) {
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

    inline bool operator!() const { return !d; }

    inline void swap(ExplicitlySharedDataPointer &other)
    { qSwap(d, other.d); }

    /* custom method */
    inline bool isNull() const { return (d == 0); }
    /* custom conversion */
    template <class U> friend class ExplicitlySharedDataPointer;

    template <class U>
    static ExplicitlySharedDataPointer<T> staticCast( const ExplicitlySharedDataPointer<U>& o ) {
        return ExplicitlySharedDataPointer<T>( static_cast<T *>( o.d ) );
    }

protected:
    T *clone();

private:
    void detach_helper();

    T *d;
};

template <class T>
Q_INLINE_TEMPLATE T *ExplicitlySharedDataPointer<T>::clone()
{
    return new T(*d);
}

template <class T>
Q_OUTOFLINE_TEMPLATE void ExplicitlySharedDataPointer<T>::detach_helper()
{
    T *x = clone();
    x->ref.ref();
    if (!d->ref.deref())
        delete d;
    d = x;
}

template <class T>
Q_INLINE_TEMPLATE ExplicitlySharedDataPointer<T>::ExplicitlySharedDataPointer(T *adata) : d(adata)
{ if (d) d->ref.ref(); }

template <class T>
Q_INLINE_TEMPLATE void qSwap(ExplicitlySharedDataPointer<T> &p1, ExplicitlySharedDataPointer<T> &p2)
{ p1.swap(p2); }

#endif // _SHARED_DATA_H_
