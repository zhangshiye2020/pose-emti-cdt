# pose-emti-cdt
CLion到期，不是自己电脑不想续，改用eclipse cdt

## Update 10.11

eclipse cdt 太难用了，用回CLion，本仓库废弃

## Windows
5,6步只针对动态链接库，静态无需配置环境
**环境&运行：**
1. TDM-GCC64/MingW64工具链，下载OpenCV源码自行编译，编译过程参考网上相关教程。
2. Eclipse->project->properties->C/C++ Build->Setting->GCC C++ Compiler->Include->Include Path(-I)设置头文件路径
3. Eclipse->project->properties->C/C++ Build->Setting->GCC C Compiler->Include->Include Path(-I)设置头文件路径
4. Eclipse->project->properties->C/C++ Build->Setting->GCC C++ Linker->Libraries->Libraries(-l)填写相关库名称，Libraries search path(-L)填写库路径
5. 这时候编译应该已经不成问题，但还是无法运行，提示返回-1073741515，在文件夹中直接点击运行提示找不到库，这时需要把库加入环境变量
6. 直接在系统中把库文件位置加入环境变量或者 Eclipse->project->properties->C/C++ Build->Environment在PATH里面将${OpenCV_DIR}/x64/mingw/bin加入环境变量

## Ubuntu
该步骤与Windows相似，不同之处在于不需要配置Libraries search path(-L)以及上述5,6步的环境变量，通过apt安装的opencv库文件将会直接放在系统库目录下，不需要额外配置
**环境&运行：**
1. 需要ubuntu安装opencv4,pkg-config,build-essential等
2. `pkg-config --cflags opencv4` 找到头文件，配置eclipse的include
3. `pkg-config --libs opencv4` 找到库名称，配置eclipse的libs

## 头文件(pkg-config --cflags opencv4所得)
```c
/usr/include/opencv
/usr/include/opencv2
```
## 库(pkg-config --libs opencv4所得)
```c
-lopencv_stitching -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_dnn_objdetect -lopencv_dnn_superres -lopencv_dpm -lopencv_highgui -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_hfs -lopencv_img_hash -lopencv_line_descriptor -lopencv_quality -lopencv_reg -lopencv_rgbd -lopencv_saliency -lopencv_shape -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_superres -lopencv_optflow -lopencv_surface_matching -lopencv_tracking -lopencv_datasets -lopencv_text -lopencv_dnn -lopencv_plot -lopencv_ml -lopencv_videostab -lopencv_videoio -lopencv_viz -lopencv_ximgproc -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_calib3d -lopencv_imgcodecs -lopencv_features2d -lopencv_flann -lopencv_xphoto -lopencv_photo -lopencv_imgproc -lopencv_core
```

