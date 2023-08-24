# Candy

中文 | [English](README_en.md)

一款基于 WebSocket 和 TUN 的 Linux VPN

这个项目旨在解决传统 VPN 流量被防火墙轻易识别和屏蔽的问题.

## 使用方法

你只需一个命令,就可以加入我们的虚拟私人网络.

```bash
podman run --rm --privileged=true --net=host --device /dev/net/tun docker.io/lanthora/candy:latest
```

如果你想搭建自己的虚拟私人网络也很简单.具体步骤请参考帮助文档.

```bash
podman run --rm docker.io/lanthora/candy:latest --help
```

我们也欢迎你加入我们的 [Telegram Group](https://t.me/+xR4K-Asvjz0zMjU1), 和我们分享你的反馈意见.
