/*!
    \file thinning.h
    \author wuciawe <wuciawe@gmail.com>
    \date 2014/10/29

 */

#ifndef THINNING_H
#define THINNING_H

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <boost/dynamic_bitset.hpp>
#include <cstdlib>
#include <pthread.h>

struct inner_iter_data {
    inner_iter_data(std::vector< boost::dynamic_bitset<> > &e1, std::vector< boost::dynamic_bitset<> > &e2, int i, int w):im(e1), marker(e2), iter(i), width(w){}
    std::vector< boost::dynamic_bitset<> > &im;
    std::vector< boost::dynamic_bitset<> > &marker;
    std::vector<int> idxs;
    int iter;
    int width;
};

void *innter_iter(void *args){
    bool p2, p3, p4, p5, p6, p7, p8, p9;
    int C, N1, N2, N, m;
    struct inner_iter_data *myArgs;
    myArgs = (struct inner_iter_data *) args;
    for(auto i : myArgs->idxs){
        for(int j = 1; j < myArgs->width; ++j){
            if(myArgs->im[i].test(j)) {
                p2 = myArgs->im[i - 1][j];
                p3 = myArgs->im[i - 1][j + 1];
                p4 = myArgs->im[i][j + 1];
                p5 = myArgs->im[i + 1][j + 1];
                p6 = myArgs->im[i + 1][j];
                p7 = myArgs->im[i + 1][j - 1];
                p8 = myArgs->im[i][j - 1];
                p9 = myArgs->im[i - 1][j - 1];

                C = (!p2 & (p3 | p4)) + (!p4 & (p5 | p6)) +
                        (!p6 & (p7 | p8)) + (!p8 & (p9 | p2));
                N1 = (p9 | p2) + (p3 | p4) + (p5 | p6) + (p7 | p8);
                N2 = (p2 | p3) + (p4 | p5) + (p6 | p7) + (p8 | p9);
                N = N1 < N2 ? N1 : N2;
                m = myArgs->iter == 0 ? ((p6 | p7 | !p9) & p8) : ((p2 | p3 | !p5) & p4);

                if (C == 1 && (N >= 2 && N <= 3) & m == 0) {
                    myArgs->marker[i].set(j);
                }
            }
        }
    }
}

void *innter_iter_fin(void *args){
    struct inner_iter_data *myArgs;
    myArgs = (struct inner_iter_data *) args;
    for(auto i : myArgs->idxs){
        myArgs->im[i] &= ~myArgs->marker[i];
    }
}

struct reset_data {
    reset_data(std::vector< boost::dynamic_bitset<> > &e1, std::vector< boost::dynamic_bitset<> > &e2, int w):image(e1), prev(e2), width(w){}
    std::vector< boost::dynamic_bitset<> > &image;
    std::vector< boost::dynamic_bitset<> > &prev;
    std::vector<int> idxs;
    int width;
};

void *reset_prev(void *args){
    struct reset_data *myArgs;
    myArgs = (struct reset_data *) args;
    for(auto i : myArgs->idxs){
        for (int j = 0; j < myArgs->width; j++)
        {
            myArgs->prev[i][j] = myArgs->image[i][j];
        }
    }
}

class TheSkeleton{
public:
    TheSkeleton(std::vector< boost::dynamic_bitset<> > & resource){
        _get_image = false;
        _is_thinned = false;

        width = resource[0].size();
        height = resource.size();

        image = resource;
        prev = resource;

    }

    TheSkeleton(const char * imagelocation) {
        _is_thinned = false;
        _get_image = false;
        cv::Mat img = cv::imread(imagelocation, CV_8UC1);
        if(!img.data){
            std::cerr << "Could not open image at " << imagelocation << std::endl;
            exit(1);
        }
        img = img < 128;

        width = img.cols;
        height = img.rows;

        image.resize(height, boost::dynamic_bitset<>(width));
        prev.resize(height, boost::dynamic_bitset<>(width));

        for(int j = 0; j < height; ++j){
            for(int i = 0; i < width; ++i){
                if(255 == img.at<uchar>(j, i)){
                    image[j].set(i);
                }
            }
        }
    }

    bool thin(int threadNum = 2, int maxIters = 0){
        if(!_is_thinned){

            pthread_t threads[threadNum];
            struct reset_data ttd(image, prev, width);
            std::vector<struct reset_data> td(threadNum, ttd);
            for(int i = 1; i < height; ++i){
                td[i % threadNum].idxs.push_back(i);
            }

            int cnt = 0;
            bool flag;

            do {
                flag = false;
                thinningGHI(image, threadNum, 0);
                thinningGHI(image, threadNum, 1);

                for (int i = 1; i < height - 1; i++)
                {
                    if(image[i] != prev[i]){
                        flag = true;
                        break;
                    }
                }
                if(flag){
                    for(int i = 0; i < threadNum; ++i){
                        pthread_create(&threads[i], NULL, reset_prev, (void *) &td[i]);
                    }

                    for(int i= 0; i < threadNum; ++i){
                        pthread_join(threads[i], NULL);
                    }
                }
            } while (flag && (maxIters == 0 || cnt++ < maxIters));

            prev.clear();
            _is_thinned = true;
            return true;
        } else {
            return true;
        }
    }

    cv::Mat getThinnedImage(){
        std::cout << res.data << std::endl;
        if(_get_image)
            return res;
        bool isSuccess = _is_thinned;
        if(!_is_thinned){
            isSuccess = thin();
        }
        if(isSuccess){
            res = cv::Mat(height, width, CV_8UC1);
            for(int j = 0; j < height; ++j){
                for(int i = 0; i < width; ++i){
                    if(image[j][i])
                        res.at<uchar>(j, i) = 255;
                    else
                        res.at<uchar>(j, i) = 0;
                }
            }
            _get_image = true;
            return res;
        } else {
            std::cerr << "Something Wrong" << std::endl;
            exit(1);
        }
    }

private:
    bool _is_thinned;
    bool _get_image;
    std::vector< boost::dynamic_bitset<> > image;
    std::vector< boost::dynamic_bitset<> > prev;
    int width;
    int height;
    cv::Mat res;
    void thinningGHI(std::vector< boost::dynamic_bitset<> > & im, int threadNum, int iter){
        std::vector< boost::dynamic_bitset<> > marker(height, boost::dynamic_bitset<>(width));
        pthread_t threads[threadNum];
        struct inner_iter_data ttd(im, marker, iter, width);
        std::vector<struct inner_iter_data> td(threadNum, ttd);
        for(int i = 1; i < height; ++i){
            td[i % threadNum].idxs.push_back(i);
        }
        for(int i = 0; i < threadNum; ++i){
            pthread_create(&threads[i], NULL, innter_iter, (void *) &td[i]);
        }

        for(int i= 0; i < threadNum; ++i){
            pthread_join(threads[i], NULL);
        }

        for(int i = 0; i < threadNum; ++i){
            pthread_create(&threads[i], NULL, innter_iter_fin, (void *) &td[i]);
        }

        for(int i= 0; i < threadNum; ++i){
            pthread_join(threads[i], NULL);
        }
    }
};



#endif