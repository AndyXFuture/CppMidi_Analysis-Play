#CPP_MIDI_Analysis-Play

​ **——使用 C++ 在 Windows 下 Visual Stduio2019 中写的一个MIDI文件的实时解析和播放**

## 关于该项目
想在单片机中部署这么一个东西，找了网上各种资料，综合考虑后还是决定自己写，所以先在Windows下验证

---

## 项目文件说明
* `Software`：里面是我写的两个程序，一个是主要MIDI解析和播放程序`MIDI_Analysis`，另一个是`toHexTxt`将文件转化成XX XX这样形式的txt文件方便自己的去写注释解读协议

* `NeedInstall`：需要安装的2.3M的一个小东西，另外一个sf2文件则是音色库

* `Document`：里面有一个我写的MIDI注释，解析的是从一个群里下的A叔改编的unravel，另一个则是参考资料

## 使用说明
项目运行在WIN11下的Visual Studio2019，通过windows库的MIDI的API来发送MIDI消息，但它不能直接发声，所以需要安装VirtualMIDISynth，[这是它的官网](http://coolsoft.altervista.org/it/virtualmidisynth#download)，音色库则在安装完后打开软件-右键右下角的图标-配置-点+号添加即可

Visual Studio2019下的C/C++桌面开发环境配置在这我就不多说了，网上一搜一大把

以上操作完后直接打开.sln文件运行即可

关于MIDI内容的解析，我还会在B站出一篇专栏来说，到时候在把链接放过来


## 参考文献
MIDI文件格式解析https://www.jianshu.com/p/59d74800b43b
MIDI tick与绝对时间的换算
https://wenku.baidu.com/view/9b6871f7f90f76c661371ac4.html
（百度文库的免费文档，真的是一股清流啊）
windows的MIDI-API调用
https://blog.csdn.net/qq_39096769/article/details/105187079

