#include <strings.h>
#include <pthread.h>
#include "common.h"


struct patient {
    char *name;
    char *q1;
    char *q2;
    char *q3;
    char *date;
    char *time;
};

uint8_t recvline[MAXLINE+1];
uint8_t buff[MAXLINE+1];

void *handle_connection(void *client_socket);

int main(int argc, char **argv) {
    
    int listenfd,connfd;
    struct sockaddr_in servaddr;
    
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("socket error.");
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);
    
    if((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0)
        err_n_die("bind error.");
    
    if((listen(listenfd, 10)) < 0)
        err_n_die("listen error.");
    
    for( ; ;) {
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_address[MAXLINE+1];
        
        
        printf("waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);
        connfd = accept(listenfd, (SA *) &addr, &addr_len);
        
        inet_ntop(AF_INET, &addr, client_address, MAXLINE);
        printf("Client connection: %s\n", client_address);
        
        pthread_t t;
        int *pclient = malloc(sizeof(int));
        *pclient = connfd;
        pthread_create(&t, NULL, handle_connection, pclient);
        
        
        
        }
    }
    
    void *handle_connection(void *client_socket) {
        int n;
        
        int connfd = *((int*) client_socket);
        free(client_socket);
        while ((n = read(connfd, recvline, MAXLINE-1)) > 0) {
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline);
            
            if(recvline[n-1] == '\n') {
                break;
            }
            memset(recvline, 0, MAXLINE);
        }
        if (n < 0)
            err_n_die("read error");
        
        
        
        // printf("%s\n", recvline);
        
        char *chatstart = "<p>Chatbot: Hello, ";
        char *chatend = "!</p>\r\n";
        
        
        if (strstr((const char*)recvline, "favicon") != NULL) {
            // contains
        }
        else if(strstr((const char*)recvline, "GET /?name") != NULL) {
            char webpage[10000];
            
            char *req = (char*)recvline;
            char delim[] = "=";
            char delim2[] = " ";
            
            char *tempptr = strtok(req, delim);
            tempptr = strtok(NULL, delim2);
            
            
            char *name = tempptr;
            
            char *t;
            char tt[strlen(name)];
            if (strstr(name, "+") != NULL) {
                int i = 0;
                for (t = name; *t != '\0'; t++) {
                    if(*t == '+') {
                        tt[i] = ' ';
                    }
                    else {
                        tt[i] = *t;
                    }
                    i++;
                }
                name = tt;
            }
            
            
            sprintf(webpage, "HTTP/1.0 200 OK\r\n\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                    "<style>\r\n"
                    "body {background-color: powderblue;"
                    "text-align: center;"
                    "border: 3px solid blue;}\r\n"
                    "form {display: inline-block;}\r\n"
                    "</style>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "Chatbot: <p id=\"cb\">Hello, %s. I would like to ask you some questions. Have you been outside your state for the past week?</p>\r\n"
                    "%s:<p\"pat\"></p>\r\n"
                    "<input id=\"resp\">\r\n"
                    "<form>\r\n"
                    "<input style=\"display:none\" id=\"am\" name=\"ap\" type=\"radio\">AM<br>\r\n"
                    "<input style=\"display:none\" id=\"pm\" name=\"ap\" type=\"radio\">PM\r\n"
                    "</form>\r\n"
                    "<button id=\"enter\" onClick=\"myFunction()\">Enter</button>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<input style=\"display:none\" id=\"q1\" type=\"text\" name=\"q1\"/>\r\n"
                    "<input style=\"display:none\" id=\"q2\" type=\"text\" name=\"q2\"/>\r\n"
                    "<input style=\"display:none\" id=\"q3\" type=\"text\" name=\"q3\"/>\r\n"
                    "<input style=\"display:none\" id=\"date\" type=\"text\" name=\"date\"/>\r\n"
                    "<input style=\"display:none\" id=\"time\" type=\"text\" name=\"time\"/>\r\n"
                    "<input style=\"display:none\" id=\"name\" type=\"text\" name=\"name\"/>\r\n"
                    "<input style=\"display:none\" id=\"next\" type=\"submit\" value=\"Next\"/>\r\n"
                    "</form>\r\n"
                    "<script type=\"text/javascript\">\r\n"
                    "var i = 0;\r\n"
                    "var mess = [\"Have you been feeling sick/feverish in general recently?\", \"Have you had cold/flu like symptoms recently?\", \"What date would you like your appointment? (format: mmddyyyy)\", \"What time would you like on that day? (format ex: 10 AM or 9 PM)\", \"Thank you for your time! Have a nice day!\"];\r\n"
                    "var apprResp = [\"yes\", \"no\", \"y\", \"n\"];\r\n"
                    "function myFunction() {\r\n"
                    "document.getElementById(\"name\").value = \"%s\";\r\n"
                    "if (i <= 0) {\r\n"
                    "if (checkValid(document.getElementById(\"resp\").value)) {\r\n"
                    "document.getElementById(\"q1\").value = document.getElementById(\"resp\").value; \r\n"
                    "document.getElementById(\"cb\").innerHTML = mess[i];\r\n"
                    "i++\r\n"
                    "}\r\n"
                    "}\r\n"
                    "else if (i == 1) {\r\n"
                    "if (checkValid(document.getElementById(\"resp\").value)) {\r\n"
                    "document.getElementById(\"q2\").value = document.getElementById(\"resp\").value; \r\n"
                    "document.getElementById(\"cb\").innerHTML = mess[i];\r\n"
                    "i++\r\n"
                    "}\r\n"
                    "}\r\n"
                    "else if (i == 2) {\r\n"
                    "if (checkValid(document.getElementById(\"resp\").value)) {\r\n"
                    "document.getElementById(\"q3\").value = document.getElementById(\"resp\").value; \r\n"
                    "document.getElementById(\"cb\").innerHTML = mess[i];\r\n"
                    "i++\r\n"
                    "}\r\n"
                    "}\r\n"
                    "else if (i == 3) {\r\n"
                    "if (checkValid3(document.getElementById(\"resp\").value)) {\r\n"
                    "document.getElementById(\"date\").value = document.getElementById(\"resp\").value; \r\n"
                    "document.getElementById(\"cb\").innerHTML = mess[i];\r\n"
                    "i++\r\n"
                    "document.getElementById(\"am\").style.display = \"block\";\r\n"
                    "document.getElementById(\"pm\").style.display = \"block\";\r\n"
                    "}\r\n"
                    "}\r\n"
                    "else {\r\n"
                    "var wu = document.getElementById(\"am\")\r\n"
                    "var vu = document.getElementById(\"pm\")\r\n"
                    "if (checkValid2(document.getElementById(\"resp\").value, wu, vu)) {\r\n"
                    "document.getElementById(\"time\").value = document.getElementById(\"resp\").value; \r\n"
                    "if(wu.checked === true) {\r\n"
                    "document.getElementById(\"time\").value += \"AM\"\r\n"
                    "}\r\n"
                    "else {\r\n"
                    "document.getElementById(\"time\").value += \"PM\"\r\n"
                    "}\r\n"
                    "document.getElementById(\"cb\").innerHTML = mess[i];\r\n"
                    "document.getElementById(\"enter\").disabled = true;\r\n"
                    "document.getElementById(\"am\").style.display = \"none\";\r\n"
                    "document.getElementById(\"pm\").style.display = \"none\";\r\n"
                    "document.getElementById(\"next\").style.display = \"block\";\r\n"
                    "}\r\n"
                    "}\r\n"
                    "} \r\n"
                    "function checkValid(x) {\r\n"
                    "for(var j = 0; j < apprResp.length; j++) {\r\n"
                    "if(x.toLowerCase() === apprResp[j]) {\r\n"
                    "return true;\r\n"
                    "}\r\n"
                    "}\r\n"
                    "alert(\"Invalid input!\");\r\n"
                    "return false;\r\n"
                    "}\r\n"
                    "function checkValid2(y, w, v) {\r\n"
                    "if(y.length <= 2 && y.length > 0) {\r\n"
                    "if(y.length === 2 && y.charAt(0) === '0') {\r\n"
                    "alert(\"Invalid input!\");\r\n"
                    "return false;\r\n"
                    "}\r\n"
                    "if((w.checked === false && v.checked == false) || parseInt(y) > 12) {\r\n"
                    "alert(\"Invalid input!\");\r\n"
                    "return false;\r\n"
                    "}\r\n"
                    "return true;\r\n"
                    "}\r\n"
                    "else {\r\n"
                    "alert(\"Invalid input!\");\r\n"
                    "return false;\r\n"
                    "}\r\n"
                    "}\r\n"
                    "function checkValid3(z) {\r\n"
                    "if(z.length === 8) {\r\n"
                    "return true;\r\n"
                    "}\r\n"
                    "else {\r\n"
                    "alert(\"Invalid input!\");\r\n"
                    "return false;\r\n"
                    "}\r\n"
                    "}\r\n"
                    "</script>\r\n"
                    "</body>\r\n"
                    "</html>\r\n", name, name, name);
            
            
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
            // strcpy(webpage, "");
        }
        
        else {
            char webpage[] = "HTTP/1.0 200 OK\r\n\r\n"
            "<!DOCTYPE html>\r\n"
            "<html>\r\n"
            "<head>\r\n"
            "<title>Coronavirus ChatBot Scheduler</title>\r\n"
            "<style>\r\n"
            "body {background-color: powderblue;}\r\n"
            "</style>\r\n"
            "</head>\r\n"
            "<body>\r\n"
            "<h1>Coronavirus ChatBot Scheduler</h1>\r\n"
            "<p>If you want to set up an appointment for a COVID-19 test, please log in your full name.</p>\r\n"
            "<form method=\"GET\">\r\n"
            "<input type=\"text\" name=\"name\"/>\r\n"
            "<input id=\"get\" type=\"submit\" value=\"submit\" />\r\n"
            "</form>\r\n"
            "</body>\r\n"
            "</html>\r\n";
            
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
        }
        return NULL;
        
    
    
    
}


