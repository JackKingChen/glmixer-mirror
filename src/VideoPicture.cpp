

extern "C"
{
#include <libavcodec/avcodec.h>
}

#include "VideoPicture.h"

#ifdef Q_OS_UNIX
#include <sys/mman.h>
#endif

// memory map
QList<VideoPicture::PictureMap*> VideoPicture::_pictureMaps;
QMutex VideoPicture::VideoPictureMapLock;
long int VideoPicture::PictureMap::_totalmemory = 0;

VideoPicture::PictureMap::PictureMap(int pageSize) : _pageSize(pageSize), _isFull(false)
{

#ifdef Q_OS_UNIX
    _map = (uint8_t *) mmap(0, PICTUREMAP_SIZE * _pageSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);

    _totalmemory += PICTUREMAP_SIZE * _pageSize;

#ifdef VIDEOPICTURE_DEBUG
    qDebug()<< "GLMixer" << QChar(124).toLatin1() << QObject::tr("Video File Picture Maps size = %1 Mb.").arg((float)_totalmemory / (float) MEGABYTE);
#endif

#endif

    int j=0;
    for (j=0; j<PICTUREMAP_SIZE;++j){
        _picture[j] = 0;
    }
}

VideoPicture::PictureMap::~PictureMap()
{
#ifdef Q_OS_UNIX
    munmap(_map, PICTUREMAP_SIZE * _pageSize);
    _totalmemory -= PICTUREMAP_SIZE * _pageSize;

#ifdef VIDEOPICTURE_DEBUG
    qDebug()<< "GLMixer" << QChar(124).toLatin1() << QObject::tr("Video File Picture Maps size = %1 Mb.").arg((float)_totalmemory / (float) MEGABYTE);
#endif

#endif
}

void VideoPicture::PictureMap::freePictureMemory(uint8_t *p)
{
    int j=0;
    for (j=0;j<PICTUREMAP_SIZE;++j){
        if (_picture[j] == p) {
            _picture[j] = NULL;
            break;
        }
    }
}

bool VideoPicture::PictureMap::isEmpty()
{
    int j=0;
    for (j=0;j<PICTUREMAP_SIZE;++j){
        if (_picture[j] != NULL)
           return false;
    }
    return true;
}

bool VideoPicture::PictureMap::isFull()
{
    int j=0;
    for (j=0;j<PICTUREMAP_SIZE;++j){
        if (_picture[j] == NULL)
           return false;
    }
    return true;
}

uint8_t *VideoPicture::PictureMap::getAvailablePictureMemory()
{

    int j=0;
    for (j=0;j<PICTUREMAP_SIZE;++j){
        if (_picture[j] == NULL) {
            _picture[j] = _map + j * _pageSize;
            return _picture[j];
        }
    }

    return NULL;
}

VideoPicture::PictureMap *VideoPicture::getAvailablePictureMap(int w, int h, enum AVPixelFormat format) {
    PictureMap *m = NULL;
    int pageSize = 0;
    if(format==AV_PIX_FMT_RGB24)
        pageSize = w * h * 3 * sizeof(uint8_t);
    else
        pageSize = w * h * 4 * sizeof(uint8_t);

    QList<PictureMap*>::iterator it = _pictureMaps.begin();
    while (it != _pictureMaps.end() ) {
        if ( (*it)->getPageSize() == pageSize && !(*it)->isFull() ) {
            m = *it;
            break;
        }
        it++;
    }

    // nothing found
    if (m == NULL) {
        m = new PictureMap(pageSize);
        VideoPicture::_pictureMaps.append(m);
#ifdef VIDEOPICTURE_DEBUG
        qDebug() << "VideoPicture::_pictureMaps size = " << _pictureMaps.size();
#endif
    }

    return m;
}


void VideoPicture::clearPictureMaps()
{
    while (!_pictureMaps.isEmpty())
         delete _pictureMaps.takeFirst();

#ifdef VIDEOPICTURE_DEBUG
    qDebug()<< "GLMixer" << QChar(124).toLatin1() << QObject::tr("Video File Picture Map cleared.");
#endif
}

void VideoPicture::freePictureMap(PictureMap *pmap)
{
    int i = _pictureMaps.indexOf(pmap);
    if (i != -1 && _pictureMaps.at(i)->isEmpty() ) {
        _pictureMaps.removeAt(i);
        delete pmap;
#ifdef VIDEOPICTURE_DEBUG
        qDebug() << "VideoPicture::_pictureMaps size = " << _pictureMaps.size();
#endif
    }
}

VideoPicture::VideoPicture(SwsContext *img_convert_ctx, int w, int h,
        enum AVPixelFormat format, bool rgba_palette) : pts(0), width(w), height(h), convert_rgba_palette(rgba_palette),  pixel_format(format),  img_convert_ctx_filtering(img_convert_ctx), action(0)
{
    for(int i=0; i<AV_NUM_DATA_POINTERS; ++i) {
        rgb.data[i] = NULL;
        rgb.linesize[i] = 0;
    }

#ifdef Q_OS_UNIX

    VideoPicture::VideoPictureMapLock.lock();
    do {
        _pictureMap = VideoPicture::getAvailablePictureMap(width, height, format);
        rgb.data[0] = _pictureMap->getAvailablePictureMemory();
    } while (rgb.data[0] == NULL);
    VideoPicture::VideoPictureMapLock.unlock();


    rgb.linesize[0] = ( format == AV_PIX_FMT_RGB24 ? 3 : 4 ) * width;

#else
    avpicture_alloc(&rgb, pixel_format, width, height);
#endif

    // initialize buffer if no conversion context is provided
    if (!img_convert_ctx_filtering) {
        int nbytes = avpicture_get_size(pixel_format, width, height);
        for(int i = 0; i < nbytes; ++i)
            rgb.data[0][i] = 0;
    }
}

VideoPicture::~VideoPicture()
{
#ifdef Q_OS_UNIX

    VideoPicture::VideoPictureMapLock.lock();
    _pictureMap->freePictureMemory(rgb.data[0]);
    VideoPicture::freePictureMap(_pictureMap);
    VideoPicture::VideoPictureMapLock.unlock();

#else
    avpicture_free(&rgb);
    rgb.data[0] = NULL;
#endif

}

void VideoPicture::saveToPPM(QString filename) const
{
    if (pixel_format != AV_PIX_FMT_RGBA)
        {
                FILE *pFile;
                int y;

                // Open file
                pFile = fopen(filename.toUtf8().data(), "wb");
                if (pFile == NULL)
                        return;

                // Write header
                fprintf(pFile, "P6\n%d %d\n255\n", width, height);

                // Write pixel data
                for (y = 0; y < height; y++)
                        fwrite(rgb.data[0] + y * rgb.linesize[0], 1, width * 3, pFile);

                // Close file
                fclose(pFile);
        }
}

void VideoPicture::fill(AVFrame *frame, double timestamp)
{
    // remember pts
    pts = timestamp;

    // ignore null frame
    if (!frame)
        return;

    if (img_convert_ctx_filtering && !convert_rgba_palette)
    {
        // Convert the image with ffmpeg sws
        if ( 0 == sws_scale(img_convert_ctx_filtering, frame->data, frame->linesize, 0,
                  height, (uint8_t**) rgb.data, (int *) rgb.linesize) )
            // fail : set pointer to NULL (do not display)
            rgb.data[0] = NULL;

        }
        // I reimplement here sws_convertPalette8ToPacked32 which does not work with alpha channel (RGBA)...
        else
        {
                // get pointer to the palette
        uint8_t *palette = frame->data[1];
                if ( palette != 0 ) {
                        // clear RGB to zeros when alpha is 0 (optional but cleaner)
                        for (int i = 0; i < 4 * 256; i += 4)
                        {
                                if (palette[i + 3] == 0)
                                        palette[i + 0] = palette[i + 1] = palette[i + 2] = 0;
                        }
                        // copy BGR palette color from frame to RGBA buffer of VideoPicture
            uint8_t *map = frame->data[0];
                        uint8_t *bgr = rgb.data[0];
                        for (int y = 0; y < height; y++)
                        {
                                for (int x = 0; x < width; x++)
                                {
                                        *bgr++ = palette[4 * map[x] + 2]; // B
                                        *bgr++ = palette[4 * map[x] + 1]; // G
                                        *bgr++ = palette[4 * map[x]];     // R
                    if (pixel_format == AV_PIX_FMT_RGBA)
                                                *bgr++ = palette[4 * map[x] + 3]; // A
                                }
                map += frame->linesize[0];
                        }
                }
        }

}




