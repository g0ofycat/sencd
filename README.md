# sencd

**sencd** *(Server Encryption Daemon; pronounced as "Second-D")* is a tool that allows you to reroute traffic, mainly from the internet, through a secure, encrypted tunnel to another server (e.g. a laptop running an Ubuntu Server) to help protect your privacy or to mask your public IP address

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

## Client Support

sencd's client supports both Linux and Windows

## Basic Usage

### Server

After configuring the server and installing all dependencies, you can easily set up the server

To start the server, you can compile from source using **git clone** (Recommended), or download the server executable from the *Releases* section. Then, you can run the **start** command

```
sencd-server start
```

This automatically starts the server environment, so by default no output would display apart from the initial server connection messages, this behavior can be overridden by using the **idle** sub-command which puts you into read-only mode while also listening to server output

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

---

## Dependencies

This project uses **cJSON** for storing minimal data and configurations, **libsodium** for packet and data encryption, and **zstd** for data compression. Ensure you have everything installed before building the project. Alternatively, prebuilt executables can be found in the **Releases** section.
