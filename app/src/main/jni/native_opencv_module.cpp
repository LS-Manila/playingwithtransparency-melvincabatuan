#include <jni.h>

#include <android/log.h>
#include <android/bitmap.h>

#include "opencv2/imgproc.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/video.hpp"

#define  LOG_TAG    "Transparency"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif



/*   Global Parameters  */

std::vector<cv::Mat> channels(4); // BGRA Image



JNIEXPORT void JNICALL
Java_ph_edu_dlsu_transparency_CameraActivity_process(JNIEnv *env, jobject instance,
                                                jobject pTarget, jbyteArray pSource, jint opacity) {
    double start;

    AndroidBitmapInfo bitmapInfo;
    uint32_t *bitmapContent;

    if (AndroidBitmap_getInfo(env, pTarget, &bitmapInfo) < 0) abort();
    if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
    if (AndroidBitmap_lockPixels(env, pTarget, (void **) &bitmapContent) < 0) abort();

    /// Access source array data... OK
    jbyte *source = (jbyte *) env->GetPrimitiveArrayCritical(pSource, 0);
    if (source == NULL) abort();

    /// cv::Mat for YUV420sp source and output BGRA
    cv::Mat srcGray(bitmapInfo.height, bitmapInfo.width, CV_8UC1, (unsigned char *) source);
    cv::Mat mbgra(bitmapInfo.height, bitmapInfo.width, CV_8UC4, (unsigned char *) bitmapContent);


/***********************************************************************************************/
    /// Native Image Processing HERE...

    start = static_cast<double>(cv::getTickCount());

#if 0
    cv::flip(srcGray, srcGray, 1); // 1 = horizontal flipping
#endif


    cv::Mat A(srcGray.size(), srcGray.type(), cv::Scalar(opacity));
    cv::Mat black(srcGray.size(), srcGray.type(), cv::Scalar(0));
    cv::Mat white(srcGray.size(), srcGray.type(), cv::Scalar(255));

    channels[0] = black;         // R channel
    channels[1] = srcGray;       // G
    channels[2] = black;         // B
    //channels[3] = A;            // A : transparent = 0, opaque = 255
    channels[3] = white;

    merge(channels, mbgra);

    LOGI("Processing took %0.2lf ms.", 1000.0 * (static_cast<double>(cv::getTickCount()) - start) / cv::getTickFrequency());

    // double cv::getTickFrequency( void ); // Tick frequency in seconds as 64-bit

/************************************************************************************************/

    /// Release Java byte buffer and unlock backing bitmap
    //env-> ReleasePrimitiveArrayCritical(pSource,source,0);
    /*
     * If 0, then JNI should copy the modified array back into the initial Java
     * array and tell JNI to release its temporary memory buffer.
     *
     * */

    env->ReleasePrimitiveArrayCritical(pSource, source, JNI_COMMIT);
    /*
 * If JNI_COMMIT, then JNI should copy the modified array back into the
 * initial array but without releasing the memory. That way, the client code
 * can transmit the result back to Java while still pursuing its work on the
 * memory buffer
 *
 * */

    /*
     * Get<Primitive>ArrayCritical() and Release<Primitive>ArrayCritical()
     * are similar to Get<Primitive>ArrayElements() and Release<Primitive>ArrayElements()
     * but are only available to provide a direct access to the target array
     * (instead of a copy). In exchange, the caller must not perform blocking
     * or JNI calls and should not hold the array for a long time
     *
     */


    if (AndroidBitmap_unlockPixels(env, pTarget) < 0) abort();
}


#ifdef __cplusplus
}
#endif