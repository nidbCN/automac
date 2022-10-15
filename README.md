# automac

为了解决校园网莫名其妙 ban 掉 MAC 地址，导致路由器设备无法上网的问题，该程序可以在 ping 失败后自动更换 MAC 地址、并且重启网卡。

当无法 ping 通指定 IP 地址时，更换 MAC 地址、重启网卡并执行指定指令。

## 兼容性

更换 MAC 地址和重启网卡均使用 `ioctl` 实现，ping 使用 Linux 的 socket 完成，理论上兼容性良好。

## 使用方法

```shell
automac <interfaceName> <ipAddress> <command>
```

## 编译方法

```shell
cmake .
cd cmake-build-release
make clean && make
```

`automac` 即为编译出来的二进制文件

