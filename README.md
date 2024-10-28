# Homework 2 - Networking
## 1. `send_udp.c` 
For the `send_udp.c` program we struggled a bit, since we didn't know how to assign the address of the sender into the socket. We tried a couple of things like `bind()`, `sendto()`, and `connect()`. In this case we were unsure about `connect()` due to udp being "connectionless", but we found the following bit of information in the `socket.h` library.
```c
/* 
    Open a connection on socket FD to peer at ADDR (which LEN bytes long).
    For connectionless socket types, just set the default address to send to and the only address from which to accept transmissions.
    Return 0 on success, -1 for errors.

    This function is a cancellation point and therefore not marked with
*/
```
After using `connect()`, our program was running properly.

## 2. `receive_udp.c`
No struggles found with this one. We just performed some standard socket `bind()`ing. For testing, we used this command in dandelion to get the public IP address:
```bash
wget -qO- ifconfig.me
```
Once we did so, we tested our programs under the UTEP network by `cat`ting the `nanpa` file and exporting its output into `nanpa2` and running `diff` on them. Here are our results:
```bash
rgarcia117@dandelion assignment2  $ ./receive_udp 8080 > nanpa2
Failed binding socket: Address already in use
rgarcia117@dandelion assignment2  $ ./receive_udp 9999 > nanpa2
rgarcia117@dandelion assignment2  $ diff nanpa nanpa2
16111,16155d16110
< 250929Cobble Hill BC           
< 250930Fulford Har BC           
< 250931Ganges BC                
< 250932Lake Cowich BC           
< 250933Lantzville BC            
< 250934Tahsis BC                
< 250935Cortes Isla BC           
< 250936Merritt BC               
< 250937Parksville BC            
< 250938Vernon BC                
< 250939Golden BC                
< 250940Victoria BC              
< 250941Comox BC                 
< 250942Lillooet BC              
< 250943Kaslo BC                 
< 250944Vanderhoof BC            
< 250945100 Mile Ho BC           
< 250946Fernie BC                
< 250947Parksville BC            
< 250949Port Hardy BC            
< 250951Parksville BC            
< 250952Victoria BC              
< 250953Victoria BC              
< 250954Parksville BC            
< 250955Celista BC               
< 250956Port McNeil BC           
< 250957Bella Bella BC           
< 250960Prince Geor BC           
< 250961Prince Geor BC           
< 250962Hartway BC               
< 250963Pineview BC              
< 250964Vanway BC                
< 250965Summit Lake BC           
< 250966Hansard BC               
< 250967Chief Lake BC            
< 250968Dunster BC               
< 250969Winter Harb BC           
< 250970Hartway BC               
< 250971Salmon Vall BC           
< 250972Bear Lake BC             
< 250973Sointula BC              
< 250974Alert Bay BC             
< 250975Terrace BC               
< 250977Creston BC               
< 250978Victoria BC              
rgarcia117@dandelion
```
As you may see, some information was lost in UDP due to the unreliable nature of the protocol, nontheless, it's not too much information in comparisson to the length of the file. Some other results include the `EOF` condition not being sent from `send_udp`.

## 3. `reply_udp.c`

## 4. `send_receive_udp.c`

## 5. `tunnel_udp_over_tcp_client.c`

## 6. `tunnel_udp_over_tcp_server.c`