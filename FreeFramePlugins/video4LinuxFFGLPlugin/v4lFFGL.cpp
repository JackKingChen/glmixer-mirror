#include <FFGL.h>
#include <FFGLLib.h>

#include <cmath>

#include "v4lFFGL.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>
#include <libv4lconvert.h>

#include <pthread.h>


#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};

char             dev_name[256];
enum io_method   io = IO_METHOD_USERPTR;
int              fd = -1;
struct buffer    *buffers;
unsigned int     n_buffers;
int              force_format = false;

// BHBN
unsigned char *  _glbuffer[2];
GLuint draw_buffer = 0;
GLuint read_buffer = 1;
GLuint textureIndex = 0;
int width = 0;
int height = 0;
struct v4lconvert_data *m_convertData;
struct v4l2_format m_capSrcFormat;
struct v4l2_format m_capDestFormat;

void *update_thread();
pthread_t thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
bool stop = true;

#define FFPARAM_DEVICE 0

void errno_exit(const char *s)
{
        fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
        exit(EXIT_FAILURE);
}

int xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}


void process_image(const void *p, int size)
{

    pthread_mutex_lock( &mutex );
    GLuint tmp = read_buffer;
    read_buffer = draw_buffer;
    draw_buffer = tmp;
    pthread_mutex_unlock( &mutex );

    int s = 0;
    s = v4lconvert_convert(m_convertData, &m_capSrcFormat, &m_capDestFormat,
                            (unsigned char *)p, size,
                           (unsigned char *)_glbuffer[draw_buffer], m_capDestFormat.fmt.pix.sizeimage);

    //fprintf(stderr, "%d error v4lconvert_convert", err);
    if ( s == -1 || s != m_capDestFormat.fmt.pix.sizeimage )
            errno_exit(v4lconvert_get_error_message(m_convertData));

//    _glbuffer = (unsigned char *)p;
}

int read_frame(void)
{
        struct v4l2_buffer buf;
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("read");
                        }
                }

                process_image(buffers[0].start, buffers[0].length);
                break;

        case IO_METHOD_MMAP:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                assert(buf.index < n_buffers);

                process_image(buffers[buf.index].start, buf.bytesused);

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;

        case IO_METHOD_USERPTR:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                errno_exit("VIDIOC_DQBUF");
                        }
                }

                for (i = 0; i < n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long)buffers[i].start
                            && buf.length == buffers[i].length)
                                break;

                assert(i < n_buffers);

                process_image((void *)buf.m.userptr, buf.bytesused);

                if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                        errno_exit("VIDIOC_QBUF");
                break;
        }

        return 1;
}

void update(void)
{

    for (;;)
    {
            fd_set fds;
            struct timeval tv;
            int r;

            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);

            if (-1 == r) {
                    if (EINTR == errno)
                            continue;
                    errno_exit("select");
            }

            if (0 == r) {
                    fprintf(stderr, "select timeout\n");
                    exit(EXIT_FAILURE);
            }

            if (read_frame())
                    break;
            /* EAGAIN - continue select loop. */
    }


}


void *update_thread()
{
    for(;;) {

        update();

        if (stop)
            break;
    }

}

void stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
                        errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                                errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
                        errno_exit("VIDIOC_STREAMON");
                break;
        }
}

void uninit_device(void)
{
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
        free(_glbuffer[0]);
        free(_glbuffer[1]);

        v4lconvert_destroy(m_convertData);
}

void init_read(unsigned int buffer_size)
{
        buffers = calloc(1, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }
}

void init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support memory mapping\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
                exit(EXIT_FAILURE);
        }

        buffers = calloc(req.count, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit("mmap");
                    // TODO unmap and free if not exit
        }
}

void init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s does not support user pointer i/o\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_REQBUFS");
                }
        }

        buffers = calloc(4, sizeof(*buffers));

        if (!buffers) {
                fprintf(stderr, "Out of memory\n");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start) {
                        fprintf(stderr, "Out of memory\n");
                        exit(EXIT_FAILURE);
                }
        }

        fprintf(stderr, "init userp %d x %d \n", 4, buffer_size);
}

void init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
                if (EINVAL == errno) {
                        fprintf(stderr, "%s is no V4L2 device\n", dev_name);
                        exit(EXIT_FAILURE);
                } else {
                        errno_exit("VIDIOC_QUERYCAP");
                }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
                fprintf(stderr, "%s is no video capture device\n", dev_name);
                exit(EXIT_FAILURE);
        }

        switch (io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                        fprintf(stderr, "%s does not support read i/o\n",   dev_name);
                        exit(EXIT_FAILURE);
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                        fprintf(stderr, "%s does not support streaming i/o\n",  dev_name);
                        exit(EXIT_FAILURE);
                }
                break;
        }


        /* Select video input, video standard and tune here. */


        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {
                /* Errors ignored. */
        }


        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format) {
                fmt.fmt.pix.width       = 320;
                fmt.fmt.pix.height      = 240;
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
//                fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

                if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
                        errno_exit("VIDIOC_S_FMT");
                /* Note VIDIOC_S_FMT may change width and height. */
        } else {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == xioctl(fd, VIDIOC_G_FMT, &fmt))
                        errno_exit("VIDIOC_G_FMT");
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        width = fmt.fmt.pix.width;
        height = fmt.fmt.pix.height;

        switch (io) {
        case IO_METHOD_READ:
                init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                init_mmap();
                break;

        case IO_METHOD_USERPTR:
                init_userp(fmt.fmt.pix.sizeimage);
                break;
        }

        fprintf(stderr, "v4lconvert_create\n");

        m_convertData = v4lconvert_create(fd);

        m_capSrcFormat = fmt;
        m_capDestFormat = fmt;
        m_capDestFormat.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;


        fprintf(stderr, "v4lconvert_try_format\n");
        int err = 0;
        err = v4lconvert_try_format(m_convertData, &m_capDestFormat, &m_capSrcFormat);
        if (err == -1)
                errno_exit(v4lconvert_get_error_message(m_convertData));

        fprintf(stderr, "malloc %d ", m_capDestFormat.fmt.pix.sizeimage);
        _glbuffer[0] = malloc(m_capDestFormat.fmt.pix.sizeimage);
        _glbuffer[1] = malloc(m_capDestFormat.fmt.pix.sizeimage);

        fprintf(stderr, "init_device %d x %d : ", width, height);
        fprintf(stderr, " convert pixelformat  %c%c%c%c ",
                                        fmt.fmt.pix.pixelformat & 0xFF, (fmt.fmt.pix.pixelformat >> 8) & 0xFF,
                                        (fmt.fmt.pix.pixelformat >> 16) & 0xFF, (fmt.fmt.pix.pixelformat >> 24) & 0xFF);
        fprintf(stderr, " to  %c%c%c%c \n",
                                        m_capDestFormat.fmt.pix.pixelformat & 0xFF, (m_capDestFormat.fmt.pix.pixelformat >> 8) & 0xFF,
                                        (m_capDestFormat.fmt.pix.pixelformat >> 16) & 0xFF, (m_capDestFormat.fmt.pix.pixelformat >> 24) & 0xFF);


}

void close_device(void)
{
        if (-1 == close(fd))
                errno_exit("close");

        fd = -1;
}

void open_device(void)
{
        struct stat st;

        if (-1 == stat(dev_name, &st)) {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",  dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                fprintf(stderr, "%s is no device\n", dev_name);
                exit(EXIT_FAILURE);
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Plugin information
////////////////////////////////////////////////////////////////////////////////////////////////////

static CFFGLPluginInfo PluginInfo ( 
    video4LinuxFreeFrameGL::CreateInstance,	// Create method
    "FFGLV4L",								// Plugin unique ID
    "FFGLVideo4Linux",                      // Plugin name
    1,                                      // API major version number
    000,                                    // API minor version number
	1,										// Plugin major version number
	000,									// Plugin minor version number
    FF_SOURCE,                              // Plugin type
    "Display Video4linux input (e.g.webcams)",            // Plugin description
    "by Bruno Herbelin"                     // About
);


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Constructor and destructor
////////////////////////////////////////////////////////////////////////////////////////////////////

video4LinuxFreeFrameGL::video4LinuxFreeFrameGL()
: CFreeFrameGLPlugin()
{
	// Input properties
    SetMinInputs(0);
    SetMaxInputs(0);

    // Parameters
    SetParamInfo(FFPARAM_DEVICE, "Device", FF_TYPE_TEXT, "/dev/video0");
    sprintf(dev_name, "/dev/video0");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//  Methods
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef FF_FAIL
    // FFGL 1.5
    DWORD   video4LinuxFreeFrameGL::InitGL(const FFGLViewportStruct *vp)
#else
    // FFGL 1.6
    FFResult video4LinuxFreeFrameGL::InitGL(const FFGLViewportStruct *vp)
#endif
{

    open_device();
    init_device();
    start_capturing();


    glEnable(GL_TEXTURE);
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &textureIndex);
    glBindTexture(GL_TEXTURE_2D, textureIndex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    read_buffer = 0;
    draw_buffer = 0;
    update();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, _glbuffer[read_buffer]);

    // Start the update thread
    stop = false;
    draw_buffer = 1;
    int rc;
    if( (rc = pthread_create( &thread, NULL, &update_thread, NULL)) )
    {
       fprintf(stderr,"Thread creation failed: %d\n", rc);
    }

    return FF_SUCCESS;
}


#ifdef FF_FAIL
    // FFGL 1.5
    DWORD   video4LinuxFreeFrameGL::DeInitGL()
#else
    // FFGL 1.6
    FFResult video4LinuxFreeFrameGL::DeInitGL()
#endif
{

    stop = true;

    pthread_join( thread, NULL );

    stop_capturing();
    uninit_device();
    close_device();

    return FF_SUCCESS;
}

#ifdef FF_FAIL
    // FFGL 1.5
    DWORD   video4LinuxFreeFrameGL::SetTime(double time)
#else
    // FFGL 1.6
    FFResult video4LinuxFreeFrameGL::SetTime(double time)
#endif
{
  m_curTime = time;
  return FF_SUCCESS;
}

#ifdef FF_FAIL
    // FFGL 1.5
    DWORD	video4LinuxFreeFrameGL::ProcessOpenGL(ProcessOpenGLStruct* pGL)
#else
    // FFGL 1.6
    FFResult video4LinuxFreeFrameGL::ProcessOpenGL(ProcessOpenGLStruct *pGL)
#endif
{


  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureIndex);


  pthread_mutex_lock( &mutex );
  // get a new frame
  GLuint tmp = read_buffer;
  pthread_mutex_unlock( &mutex );


  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, _glbuffer[tmp]);


  //modulate texture colors with white (just show
  //the texture colors as they are)
  glColor4f(1.f, 1.f, 1.f, 1.f);
  //(default texturemapping behavior of OpenGL is to
  //multiply texture colors by the current gl color)
  
  glBegin(GL_QUADS);

  //lower left
  glTexCoord2d(0.0, 0.0);
  glVertex2f(-1.0,-1);

  //upper left
  glTexCoord2d(0.0, 1.0);
  glVertex2f(-1,1);

  //upper right
  glTexCoord2d(1.0, 1.0);
  glVertex2f(1,1);

  //lower right
  glTexCoord2d(1.0, 0.0);
  glVertex2f(1,-1);
  glEnd();

  //unbind the texture
  glBindTexture(GL_TEXTURE_2D, 0);

  //disable texturemapping
  glDisable(GL_TEXTURE_2D);


  return FF_SUCCESS;
}

#ifdef FF_FAIL
    // FFGL 1.5
    DWORD	video4LinuxFreeFrameGL::SetTextParameter(unsigned int index, const char *value)
#else
    // FFGL 1.6
    FFResult video4LinuxFreeFrameGL::SetTextParameter(unsigned int index, const char *value)
#endif
{
    if (index == FFPARAM_DEVICE) {

        if (stop)
            sprintf(dev_name, "%s", value);
        // TODO : stop and restart plugin if already running

        return FF_SUCCESS;
    }

    return FF_FAIL;
}


char* video4LinuxFreeFrameGL::GetTextParameter(unsigned int index)
{
    if (index == FFPARAM_DEVICE)
        return dev_name;

    return (char *)FF_FAIL;
}


