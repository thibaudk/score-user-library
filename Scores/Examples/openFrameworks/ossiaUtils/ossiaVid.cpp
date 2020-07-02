#include "ossiaVid.h"

//--------------------------------------------------------------
ofVec4f placeCanvas(const unsigned int* wAndH, const float& s, const ofVec3f& p)
{
    float width{wAndH[0] * s};
    float height{wAndH[1] * s};

    float wOfset{width / 2};
    float hOfset{height / 2};

    int ofWidth{ofGetWidth()};
    int ofHeight{ofGetHeight()};

    int ofHalfW{ofWidth / 2};
    int ofHalfH{ofHeight / 2};

    float xOfset{ofHalfW + (ofHalfW * p[0])};
    float yOfset{ofHalfH + (ofHalfH * p[1])};

    return ofVec4f{(xOfset - wOfset),
                (yOfset - hOfset),
                width,
                height};
}

//--------------------------------------------------------------
ossiaVid::ossiaVid()
{
    params.add(drawVid.set("draw_video", false));

    params.add(size.set("size", 1., 0., 10.));
    prevSize = size;

    params.add(placement.set("placement",
                             ofVec2f(0, 0),
                             ofVec2f(-1, -1),
                             ofVec2f(1, 1)));
    prevPlace = placement;

    params.add(color.set("draw_color",
                         ofVec4f(255, 255, 255, 255),
                         ofVec4f(0, 0, 0, 0),
                         ofVec4f(255, 255, 255, 255)));
}

void ossiaVid::checkResize()
{
    if (prevSize != size || prevPlace != placement)
    {
        canvas = placeCanvas(vidWandH, size, placement);
        prevSize = size;
        prevPlace = placement;
    }
}

void ossiaVid::setMatrix(ofParameterGroup& params)
{
    pixMatrix.setName("matrix");

    int i{1};
    int j{1};

    for (ofParameter<float>& p: pixVal)
    {
        p.set("columms_" + to_string(i) + "/row_" + to_string(j), 0, 0, 255);
        pixMatrix.add(p);

        j++;

        if (j > MATRIX_SIZE)
        {
            i++;
            j = 1;
        }
    }

    pixControl.setName("pixels");
    pixControl.add(lookUp.set("look_up", 0, 0, 4));
    pixControl.add(getPixels.set("get", false));
    pixControl.add(hPoints.set("horizontal_points", MATRIX_SIZE / 2, 1, MATRIX_SIZE));
    pixControl.add(vPoints.set("vertical_points", MATRIX_SIZE / 2, 1, MATRIX_SIZE));
    pixControl.add(threshold.set("threshold", 128, 1, 255));
    pixControl.add(pixMatrix);
    pixControl.add(averageColor.set("average_color",
                                    ofVec4f(255, 0, 0, 0),
                                    ofVec4f(0, 0, 0, 0),
                                    ofVec4f(255, 255, 255, 255)));
    pixControl.add(centroid.set("barycenter",
                                ofVec2f(0, 0),
                                ofVec2f(-1, -1),
                                ofVec2f(1, 1)));
    pixControl.add(drawCircles.set("draw_matrix", false));
    pixControl.add(drawCenter.set("draw_barycenter", false));
    pixControl.add(circleSize.set("circles_size", 0.1, 0, 1));
    pixControl.add(circleResolution.set("circle_resolution", 22, 3, 100));
    pixControl.add(circleColor.set("circles_color",
                                   ofVec4f(255, 255, 255, 255),
                                   ofVec4f(0, 0, 0, 0),
                                   ofVec4f(255, 255, 255, 255)));
    params.add(pixControl);
}

void ossiaVid::processPix(const ofPixels& px, ofParameter<float> *pv, const ofVec4f& canvas, const float& z, const float& size)
{
    size_t vidWidth = px.getWidth();
    size_t vidHeight = px.getHeight();
    size_t widthSpread = vidWidth / hPoints; // horizontal points
    unsigned int verticalStep = MATRIX_SIZE - vPoints; // vertical points
    size_t heightSpread = vidHeight / vPoints; // neumber of pixels between each points
    size_t widthMargin = widthSpread / 2; // minimum number of pixels from the left
    size_t heightMargin = heightSpread / 2; // minimum number of pixels from the top
    // number of skiped pixels before starting a new line
    unsigned int widthRemainder = vidWidth - (vidWidth % hPoints) + widthMargin;
    unsigned int heightRemainder = vidHeight - (vidHeight % vPoints) + heightMargin;

    ofVec4f midColor{0, 0, 0, 0};
    float lightSum{0};
    ofVec2f baryCenter{0, 0};
    unsigned int iter{0};

    ofSetCircleResolution(circleResolution);

    ofSetColor(circleColor->y,
               circleColor->z,
               circleColor->w,
               circleColor->x);

    for (size_t i = widthMargin; i < widthRemainder; i+= widthSpread)
    {
        for (size_t j = heightMargin; j < heightRemainder; j+= heightSpread)
        {
            ofColor pxColor = px.getColor(i, j);
            midColor[0] += pxColor.a;
            midColor[1] += pxColor.r;
            midColor[2] += pxColor.g;
            midColor[3] += pxColor.b;
            iter++;

            float focus{0};

            switch (lookUp)
            {
            case 0:
                focus = pxColor.getLightness();
                break;
            case 1:
                focus = 255 - pxColor.getLightness();
                break;
            case 2:
                focus = pxColor.r;
                break;
            case 3:
                focus = pxColor.g;
                break;
            case 4:
                focus = pxColor.b;
                break;
            }

            if (focus >= threshold)
            {
                pv->set(focus);

                if (drawCircles) // draw_matrix
                {
                    ofDrawCircle(canvas[0] + i * size,
                            canvas[1] + j * size,
                            z,
                            circleSize * focus); // cicle size
                }

                baryCenter += ofVec2f(i, j) * focus;
                lightSum += focus;

            } else {
                pv->set(0.f);
            }

            pv++;
        }
        pv+= verticalStep;
    }

    midColor /= iter;
    averageColor.set(midColor); // average_color

    if (lightSum != 0)
    {
        baryCenter /= lightSum;
        if (drawCenter) // draw_barycenter
        {
            ofDrawCircle((canvas[0] + baryCenter[0]) * size,
                    (canvas[1] + baryCenter[1]) * size,
                    z,
                    circleSize * size * 255); // cicle size
        }

        baryCenter[0] /= vidWidth / 2;
        baryCenter[0] -= 1;
        baryCenter[1] /= vidHeight / 2;
        baryCenter[1] -= 1;

        centroid.set(ofVec3f(baryCenter[0], baryCenter[1], 0)); // centroid
    }
}

//--------------------------------------------------------------
#ifdef CV
void ossiaCv::allocateCvImg(const unsigned int* wAndH)
{
    colorImg.allocate(wAndH[0], wAndH[1]);
    grayImage.allocate(wAndH[0], wAndH[1]);
    grayMin.allocate(wAndH[0], wAndH[1]);
    grayMax.allocate(wAndH[0], wAndH[1]);
}

//void ossiaCv::cvUpdate()
//{
//    grayMin = grayImage;
//    grayMax = grayImage;
//    grayMin.threshold(minThreshold, true);
//    grayMax.threshold(maxThreshold);
//    cvAnd(grayMin.getCvImage(), grayMax.getCvImage(), grayImage.getCvImage(), NULL);

//    grayImage.flagImageChanged();

//    contourFinder.findContours(grayImage, minArea, maxArea, 10, false);
//}
#endif

//--------------------------------------------------------------
ossiaPlayer::ossiaPlayer(string file)
    :path{file}
{
}

void ossiaPlayer::setup()
{
    vid.load(path);           // initialize video path
    vid.setLoopState(OF_LOOP_NONE);
    vid.play();
    vid.setPaused(true);

    params.setName(ofFile(path).getBaseName());    // set parameters

    play.addListener(this, &ossiaPlayer::setPlay);
    params.add(play.set("play", false));

    loop.addListener(this, &ossiaPlayer::setLoop);
    params.add(loop.set("loop", false));

    params.add(seek.set("seek", 0., 0., 1.));
    seek.addListener(this, &ossiaPlayer::setFrame);

    params.add(volume.set("volume", 1., 0., 1.));
    volume.addListener(this, &ossiaPlayer::setVolume);

    setMatrix(params);

    vidWandH[0] = vid.getWidth(); // get the video's original size
    vidWandH[1] = vid.getHeight();

#ifdef CV
    allocateCvImg(vidWandH);
#endif

    canvas = placeCanvas(vidWandH, size, placement);
}

void ossiaPlayer::update()
{
    vid.update();
}

void ossiaPlayer::setPlay(bool &toPlay)
{
    vid.setPaused(!toPlay);
}

void ossiaPlayer::setLoop(bool &toLoop)
{
    if (toLoop)
    {
        vid.setLoopState(OF_LOOP_NORMAL);
        if (vid.getIsMovieDone()) vid.setPosition(0);
    } else
    {
        vid.setLoopState(OF_LOOP_NONE);
    }
}

void ossiaPlayer::setFrame(float &toSeek)
{
    vid.setPosition(toSeek);
}

void ossiaPlayer::setVolume(float &toAmp)
{
    vid.setVolume(toAmp);
}

void ossiaPlayer::draw()
{
    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canvas[0], // getTexture allows the use of the Z axis
                canvas[1],
                placement->z,
                canvas[2],
                canvas[3]);
    }

    if (getPixels) processPix(vid.getPixels(), pixVal, canvas, placement->z, size);
}

void ossiaPlayer::close()
{
    vid.stop();
    vid.closeMovie();
}

//--------------------------------------------------------------
ossiaGrabber::ossiaGrabber(ofVideoDevice dev)
    :device{dev}
{
}

void ossiaGrabber::setup(unsigned int width, unsigned int height)
{
    vid.setDeviceID(device.id);
    vid.initGrabber(width, height);

    params.setName(device.deviceName);    // set parameters

    vidWandH[0] = vid.getWidth();
    vidWandH[1] = vid.getHeight();

#ifdef CV
    allocateCvImg(vidWandH);
#endif

    params.add(freeze.set("freeze", false));

    setMatrix(params);

    canvas = placeCanvas(vidWandH, size, placement);
}

void ossiaGrabber::update()
{
    if (!freeze) vid.update();
}

void ossiaGrabber::draw()
{
    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canvas[0], // getTexture allows the use of the Z axis
                canvas[1],
                placement->z,
                canvas[2],
                canvas[3]);
    }

    if (getPixels) processPix(vid.getPixels(), pixVal, canvas, placement->z, size);
}

void ossiaGrabber::close()
{
    vid.close();
}

//--------------------------------------------------------------
#ifdef KINECT
ossiaKinect::ossiaKinect(int dev)
    :device{dev}
{
}

void ossiaKinect::setup(bool infrared)
{
    // enable depth->video image calibration
    vid.setRegistration(true);

    vid.init(infrared);
    vid.open(device);

    params.setName(to_string(device));    // set parameters

    vidWandH[0] = vid.getWidth();
    vidWandH[1] = vid.getHeight();

#ifdef CV
    allocateCvImg(vidWandH);
#endif

    params.add(freeze.set("freeze", false));

    setMatrix(params);

    canvas = placeCanvas(vidWandH, size, placement);

    // print the intrinsic IR sensor values
    if(vid.isConnected()) {
        cout << "kinect " << device <<" sensor-emitter dist: " << vid.getSensorEmitterDistance() << "cm\n"
        << "kinect " << device <<" sensor-camera dist:  " << vid.getSensorCameraDistance() << "cm\n"
        << "kinect " << device <<" zero plane pixel size: " << vid.getZeroPlanePixelSize() << "mm\n"
        << "kinect " << device <<" zero plane dist: " << vid.getZeroPlaneDistance() << "mm\n";
    }
}

void ossiaKinect::update()
{
    if (!freeze) vid.update();
}

void ossiaKinect::draw()
{
    if (drawVid || getPixels) checkResize();

    if (drawVid)
    {
        ofSetColor(color->y,
                   color->z,
                   color->w,
                   color->x);

        vid.getTexture().draw(canvas[0], // getTexture allows the use of the Z axis
                canvas[1],
                placement->z,
                canvas[2],
                canvas[3]);
    }

    if (getPixels) processPix(vid.getPixels(), pixVal, canvas, placement->z, size);
}

void ossiaKinect::close()
{
    vid.close();
}
#endif
