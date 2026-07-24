#sencd

**sencd** *(Server Encryption Daemon; pronounced as "Second-D")* is a tool that allows you to reroute traffic, mainly from the internet, through a secure and encrypted tunnel to another server (e.x. A Laptop running a Ubuntu Server) so you can safely hide your identity

## Server Support

All servers have different routing methods in terms of sending sencd's packets to the internet. Below is a list of all available servers that sencd supports. Feel free to open a pull request if you can get other servers working with sencd

---

- Ubuntu Server

## Configuring the Server

For the server, you need to do some things to connect to your server and reroute packets to the server's connected internet, below are the required steps for all servers

- Must expose your IP to the public *(e.x. ngrok)*
- Must configure the server so that incoming packets get reroutued to the internet

Every server reroutes packets differently, either by some configuration file that handles it or built in commands. All supported servers have their guide located below on how to configure the server for packet traffic

---

### Ubuntu Server



## Client Support

sencd's client supports both Linux and Windows

## Basic Usage

### Server

After configuring the server and installing all dependencies, you can easily setup the server

To start the server, you can compile from source using **git clone** (Recommended), OR download the server executable from the *Releases* section. Then, you can run the **start** command

```
sencd-server start
```

This automatically starts the server environment, so by default no output would show apart from the initial server connection messages, this can easily be bypassed by using the **idle** sub-command which puts you in a Read-only mode while also listening to server output

```
sencd-server > idle
```

*(NOTE: This doesn't show server output messages before idle mode was activated)*. Idle mode is most recommended when you're not actively using the server environment

---

### Client

After installing all dependencies, you can connect to a server by using the **connect** command

```
sencd-client connect <optional IP> <optional port>
```

By default, this connects to **localhost (127.0.0.1)** and the **TCP port 8080**

When you first connect, you will be put in the client environment. You can easily disconnect, check server stats, and more. When you close the terminal that's running it, the daemon will still be running so you can keep connected to the server. To re-enter the client environment, you can run the **attach** command ONLY if the daemon is currently running

```
sencd-client attach
```

### Note for the client [For maintainers]

I'm currently writing this in sencd's early stages of development. Creating an "idle mode" similar to the server except on the client would be trivial. The way that idle mode and environment mode works is simply by running them in different threads and using a mutex for some basic race condition protection. The output to the client sent from the server would usually be information or some error messages which would be more useful if found in the server console. Creating an idle mode for the client would be pretty counter-intuitive since you can't run any commands, therefore you wont see any output in the first place

---

## Dependencies

This project uses **cJSON** for storing minimal data and configurations, **libsodium** for packet and data encryption, and **zstd** for data compression. Ensure you have everything install before building this project, although prebuilt executables can be found in the **Releases** section
