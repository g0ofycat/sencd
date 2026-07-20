# sencd

**sencd** *(Server Encryption Daemon; pronounced as "Second-D")* is a tool that allows you to reroute traffic, mainly from the internet, through a secure and encrypted tunnel to another server (e.x. A Laptop running a Ubuntu Server) so you can safely hide your identity.

## Server Support

All servers have different routing methods in terms of sending sencd's packets to the internet. Below is a list of all available servers that sencd supports. Feel free to open a pull request if you can get other servers working with sencd

---

- Ubuntu Server

## Client Support

sencd's client supports both Linux and Windows.

## Dependencies

This project uses **cJSON** for storing minimal data and configurations, **libsodium** for packet and data encryption, and **zstd** for data compression. Ensure you have everything install before building this project, although prebuilt executables can be found in the **Releases** section.
