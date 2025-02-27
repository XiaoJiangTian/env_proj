# env_proj
项目概述：本项目以STM32为下位机采集环境数据，RK3568开发板作为上位机服务器，使用QT编写客户端。使用阿里云实现通过客户端访问服务器获取下位机数据或设置其工作状态的功能。

技术栈：C/C++、Python、线程池、多进程、网络通信、QT、IO多路复用、Linux系统编程

底层数据采集：在STM32下使用SPI/I2C/单总线等协议对硬件模块进行驱动，实现数据采集以及多种对外通信方式。

上位机服务器：通过Epoll+线程池架构结合多进程/共享内存/条件变量处理多客户端连接以及线程资源管理。

阿里云中转站：使用阿里云物联网中的云产品流转等功能实现上下位机的通信，通过物模型实时监控环境系统。

可视化客户端：基于QT的Widget，PushButton等配合MySql以及Socket实现与服务器链接以及数据显示和设置功能。
