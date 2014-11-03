#ifdef ENABLE_VLC

#ifndef _VLC_LIB_H_
#define _VLC_LIB_H_

#include <QString>
#include <QStringList>

struct libvlc_instance_t;


class VlcLib
{
static VlcLib* INSTANCE;

public:
    VlcLib();
    ~VlcLib();

    //! returns libvlc instance object.
    static VlcLib* instance()  { return INSTANCE; }
    libvlc_instance_t* core();

    //! initialization 
    bool init();

    //! print most recent error message of libvlc
    static void print_error();
    static bool isError();

private:
    libvlc_instance_t     *m_vlcInstance;
};

#endif // _VLC_LIB_H_
#endif // ENABLE_VLC
