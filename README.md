# sencd

**sencd** *(Server Encryption Daemon; pronounced as "Second-D")* is a lightweight tool that allows you to reroute traffic, mainly from the internet, through a secure, encrypted tunnel to another server (e.g. a laptop running an Ubuntu Server) to help protect your privacy or to mask your public IP address

## Server Support

All supported servers route sencd's packets to the internet differently. Below is a list of all available servers that sencd supports. Feel free to open a pull request if you get another server working with sencd

- Ubuntu Server

## Configuring the Server

Before using the server, you must complete a few configuration steps to connect to your server and reroute packets through the server's internet connection. Below are the required steps for all servers

- Must expose your IP to the public *(e.g. ngrok)*
- Must configure the server so that incoming packets get rerouted to the internet

Every server reroutes packets differently, either through configuration files or built-in commands. All supported servers have their guide located below on how to configure packet routing

---

### Ubuntu Server

1. Expose your server to the public internet. Your sencd server needs to be reachable from wherever your client is. If you don't have a static public IP or port-forwarding access on your router, use a tunneling service like *ngrok* to expose both the TCP control port (default `8080`) and the UDP tunnel port (same port number, UDP)

2. Enable IP forwarding on the server. This tells the Linux kernel to actually forward packets between network interfaces, rather than only handling traffic addressed to itself:

```
sudo sysctl -w net.ipv4.ip_forward=1
```

To make this persist across reboots, add `net.ipv4.ip_forward=1` to `/etc/sysctl.conf` and run `sudo sysctl -p`

3. Set up NAT so tunneled traffic can reach the real internet. sencd creates a virtual network interface named `sencd0` on the server, assigned an address on the `10.8.0.0/24` subnet. Packets arriving on it need to be translated (NAT'd) out through your server's real internet-facing interface. Check yours with `ip a` - it's commonly `eth0`, but modern Ubuntu installs often use predictable names like `enp0s3` instead

```
sudo iptables -t nat -A POSTROUTING -o <your-interface> -j MASQUERADE
sudo iptables -A FORWARD -i sencd0 -o <your-interface> -j ACCEPT
sudo iptables -A FORWARD -i <your-interface> -o sencd0 -m state --state RELATED,ESTABLISHED -j ACCEPT
```

To persist these rules across reboots, install `iptables-persistent` (`sudo apt install iptables-persistent`) and save with `sudo netfilter-persistent save`

4. Note your subnet. sencd assigns your server and connected clients addresses on a private virtual subnet (default `10.8.0.0/24`) so tunneled traffic can be routed between them. You don't need to configure this manually - sencd handles interface and address assignment automatically when the tunnel starts

---

## Client Support

sencd's client supports both Linux and Windows

## Basic Usage

### Server

After configuring the server and installing all dependencies, you can easily set up the server

To start the server, you can compile from source using **git clone** (Recommended), or download the server executable from the *Releases* section. Then, you can run the **start** command

```
sencd-server start
```

This automatically starts the server environment, so by default no output would be displayed apart from the initial server connection messages; this behavior can be overridden by using the **idle** sub-command, which puts you into read-only mode while also listening to server output

```
sencd-server > idle
```

> **Note:** This doesn't show server output messages before idle mode was activated

Idle mode is most recommended when you're not actively using the server environment

---

### Client

After installing all dependencies, you can connect to a server by using the **connect** command

```
sencd-client connect <optional IP> <optional port>
```

By default, this connects to **localhost (127.0.0.1)** and the **TCP port 8080**

When you first connect, you will be put in the client environment. You can easily disconnect, check server stats, and more. Closing the terminal does not stop the daemon, so your connection remains active. To re-enter the client environment, you can run the **attach** command only if the daemon is currently running

```
sencd-client attach
```

### Note for the client [For maintainers]

I'm currently writing this in sencd's early stages of development. Implementing an "idle mode" similar to the server except on the client would be trivial. The way that idle mode and environment mode work is simply by running them in different threads and using a mutex for some basic race condition protection. Output sent from the server to the client usually consists of informational and error messages, which are generally more useful in the server console. Implementing an idle mode for the client would be counter-intuitive since you can't run any commands, therefore you won't see any output in the first place

## Dependencies

For both the server and client, this project uses **pkg-config** for managing all dependencies, **cJSON** for storing minimal data and configurations, **libsodium** for packet and data encryption, and **zstd** for data compression. Ensure you have everything installed before building the project. Alternatively, prebuilt executables can be found in the **Releases** section
