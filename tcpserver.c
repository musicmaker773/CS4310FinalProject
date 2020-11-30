#include <strings.h>
#include <pthread.h>
#include "common.h"
#include "myqueue.h"


pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int patientSize = 0;
int numOfPatients = 0;

struct patient {
    char name[60];
    char q1[4];
    char q2[4];
    char q3[4];
    char date[9];
    char time[5];
    int cancelled;
};

struct patient patients[20];



uint8_t recvline[MAXLINE+1];
uint8_t buff[MAXLINE+1];

void *handle_connection(void *client_socket);

void * thread_function(void *arg);

int main(int argc, char **argv) {
    
    int listenfd,connfd;
    struct sockaddr_in servaddr;
    
    
    for(int i=0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, thread_function, NULL);
    }
    
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
        
        
        int *pclient = malloc(sizeof(int));
        *pclient = connfd;
        pthread_mutex_lock(&mutex);
        enqueue(pclient);
        pthread_mutex_unlock(&mutex);
        // pthread_create(&t, NULL, handle_connection, pclient);
        
        
        
        }
    }
    void * thread_function(void *arg) {
        while(1) {
            pthread_mutex_lock(&mutex);
            int *pclient = dequeue();
            pthread_mutex_unlock(&mutex);
            if(pclient != NULL) {
                handle_connection(pclient);
            }
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

        
        if (strstr((const char*)recvline, "favicon.ico") != NULL) {
            // contains
            
            
        }else if(strstr((const char*)recvline, "GET /?name") != NULL) {
            char webpage[20000];
            
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
        }else if (strstr((const char*)recvline,"GET /?q1=") != NULL) {
            char *req = (char*)recvline;
        
            char delim[] = "=";
           char delim2[] = " ";
           char delim3[] = "&";

           
            char *tempptr = strtok(req, delim);
           tempptr = strtok(NULL, delim3);
           
           char *q1 = tempptr;
           
           tempptr = strtok(NULL, delim);
           tempptr = strtok(NULL, delim3);
           
           char *q2 = tempptr;
           
           tempptr = strtok(NULL, delim);
           tempptr = strtok(NULL, delim3);
           
           char *q3 = tempptr;
           
           tempptr = strtok(NULL, delim);
           tempptr = strtok(NULL, delim3);
           
           char *date = tempptr;
           
           tempptr = strtok(NULL, delim);
           tempptr = strtok(NULL, delim3);
           
           char *time = tempptr;
           
           tempptr = strtok(NULL, delim);
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
           
            int checkName = 0; // false
            int cni = 0;
           // printf("%s\n", q1);
            char altmes[50];
            
           if (patientSize == 0) {
               strcpy(patients[0].name, (const char*)name);
               strcpy(patients[0].q1, (const char*)q1);
               strcpy(patients[0].q2,(const char*)q2);
               strcpy(patients[0].q3, (const char*)q3);
               strcpy(patients[0].date,(const char*)date);
               strcpy(patients[0].time, (const char*)time);
               patients[0].cancelled = 0;
               
               // printf("%s\n", patients[0].name);
               strcpy(altmes, "");
               numOfPatients++;
               patientSize++;
           } else {
               
               for (int i = 0; i < patientSize; i++) {
                   if((strstr(patients[i].date,(const char*)date) != NULL) && (patients[i].cancelled == 0)){
                       if(strstr(patients[i].time,(const char*)time) != NULL){
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
                           "<h1>Sorry...</h1>\r\n"
                           "<p>Sorry, but this time has already been taken. Please login with your name and try again.</p>\r\n"
                           "<form method=\"GET\">\r\n"
                           "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                           "</form>\r\n"
                           "</body>\r\n"
                           "</html>\r\n";
                           
                           snprintf((char*)buff, sizeof(buff), "%s", webpage);
                           
                           write(connfd, (char*) buff, strlen((char *)buff));
                           close(connfd);
                           
                           return NULL;
                       }
                   }
                   if (strstr(patients[i].name,(const char*)name) != NULL) {
                        checkName = 1; // true
                        cni = i;
                       
                   }
                   
               }
               if (checkName == 1) {
                   
                
                   strcpy(patients[cni].q1, (const char*)q1);
                   strcpy(patients[cni].q2,(const char*)q2);
                   strcpy(patients[cni].q3, (const char*)q3);
                   strcpy(patients[cni].date,(const char*)date);
                   strcpy(patients[cni].time, (const char*)time);
                
                   if (patients[cni].cancelled == 1) {
                       patients[cni].cancelled = 0;
                       strcpy(altmes, "");
                   } else {
                       strcpy(altmes, "You have sucessfully rescheduled.");
                   }
                   
               } else {
                   
                   strcpy(patients[patientSize].name, (const char*)name);
                   strcpy(patients[patientSize].q1, (const char*)q1);
                   strcpy(patients[patientSize].q2,(const char*)q2);
                   strcpy(patients[patientSize].q3, (const char*)q3);
                   strcpy(patients[patientSize].date,(const char*)date);
                   strcpy(patients[patientSize].time, (const char*)time);
                   patients[patientSize].cancelled = 0;
                   
                   strcpy(altmes, "");
                   numOfPatients++;
                   patientSize++;
               }
               
           }
            
            printf("%d", patientSize);
            char webpage[10000];
            sprintf(webpage,"HTTP/1.0 200 OK\r\n\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                    "<style>\r\n"
                    "body {background-color: powderblue;}\r\n"
                    "</style>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "<h1>Thank you %s for your time.</h1>\r\n"
                    "<p>%s We will be seeing you at %s on %s.</p>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                    "</form>\r\n"
                    "</body>\r\n"
                    "</html>\r\n", name, altmes, time, date);
            
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
            
        }else if (strstr((const char*)recvline,"POST /") != NULL) {
            
            
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
                    "<h1>Cancel or Check Appointment</h1>\r\n"
                    "<p>Type your name and we'll see if we can check or cancel appointment.</p>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<label for=\"choice\">Choose either check or cancel:</label>\r\n"
                    "<select name=\"choice\" id=\"choice\">\r\n"
                    "<option value=\"check\">Check</option>\r\n"
                    "<option value=\"cancel\">Canel</option>\r\n"
                    "</select>\r\n"
                    "<br><br>\r\n"
                    "<input type=\"text\" name=\"name\"/>\r\n"
                    "<input id=\"get\" type=\"submit\" value=\"Next\" />\r\n"
                    "</form>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                    "</form>\r\n"
                    "</body>\r\n"
                    "</html>\r\n";
            
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
            
            
            
        }else if (strstr((const char*)recvline,"GET /?choice=cancel") != NULL) {
            
            char webpage[10000];
            char *req = (char*)recvline;
            char delim[] = "=";
            char delim2[] = " ";
            char delim3[] = "&";
            
            char *tempptr = strtok(req, delim);
            tempptr = strtok(NULL, delim3);
            tempptr = strtok(NULL, delim);
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
            for (int i = 0; i < patientSize; i++) {
                if((strstr(patients[i].name,(const char*)name) != NULL) && (patients[i].cancelled == 0)){
                    
                    patients[i].cancelled = 1;
                    numOfPatients--;
                    
                    sprintf(webpage, "HTTP/1.0 200 OK\r\n\r\n"
                            "<!DOCTYPE html>\r\n"
                            "<html>\r\n"
                            "<head>\r\n"
                            "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                            "<style>\r\n"
                            "body {background-color: powderblue;}\r\n"
                            "</style>\r\n"
                            "</head>\r\n"
                            "<body>\r\n"
                            "<h1>Okay, %s...</h1>\r\n"
                            "<p>You have sucessfully cancelled your appointment that was scheduled at %s on %s.</p>\r\n"
                            "<form method=\"GET\">\r\n"
                            "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                            "</form>\r\n"
                            "</body>\r\n"
                            "</html>\r\n", patients[i].name, patients[i].time, patients[i].date);
                    snprintf((char*)buff, sizeof(buff), "%s", webpage);
                    
                    write(connfd, (char*) buff, strlen((char *)buff));
                    close(connfd);
                    
                    return NULL;
                }
            }
            
            sprintf(webpage, "HTTP/1.0 200 OK\r\n\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                    "<style>\r\n"
                    "body {background-color: powderblue;}\r\n"
                    "</style>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "<h1>Appointment doesn't exist.</h1>\r\n"
                    "<p>Sorry, %s, but it seems we didn't find an appointment under your name to cancel.</p>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                    "</form>\r\n"
                    "</body>\r\n"
                    "</html>\r\n", name);
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
            
            return NULL;
            
            
            
            
        }
        else if (strstr((const char*)recvline,"GET /?choice=check") != NULL) {
            
            char webpage[10000];
            char *req = (char*)recvline;
            char delim[] = "=";
            char delim2[] = " ";
            char delim3[] = "&";
            
            char *tempptr = strtok(req, delim);
            tempptr = strtok(NULL, delim3);
            tempptr = strtok(NULL, delim);
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
            
            for (int i = 0; i < patientSize; i++) {
                if((strstr(patients[i].name,(const char*)name) != NULL) && (patients[i].cancelled == 0)){
                    sprintf(webpage, "HTTP/1.0 200 OK\r\n\r\n"
                            "<!DOCTYPE html>\r\n"
                            "<html>\r\n"
                            "<head>\r\n"
                            "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                            "<style>\r\n"
                            "body {background-color: powderblue;}\r\n"
                            "</style>\r\n"
                            "</head>\r\n"
                            "<body>\r\n"
                            "<h1>Okay, %s...</h1>\r\n"
                            "<p>Your appointment is at %s on %s.</p>\r\n"
                            "<form method=\"GET\">\r\n"
                            "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                            "</form>\r\n"
                            "</body>\r\n"
                            "</html>\r\n", patients[i].name, patients[i].time, patients[i].date);
                    snprintf((char*)buff, sizeof(buff), "%s", webpage);
                    
                    write(connfd, (char*) buff, strlen((char *)buff));
                    close(connfd);
                    
                    return NULL;
                }
            }
            
            sprintf(webpage, "HTTP/1.0 200 OK\r\n\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>Coronavirus ChatBot Scheduler</title>\r\n"
                    "<style>\r\n"
                    "body {background-color: powderblue;}\r\n"
                    "</style>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "<h1>Sorry...</h1>\r\n"
                    "<p>Sorry, %s, but you don't have an appointment. You can create one at the home page.</p>\r\n"
                    "<form method=\"GET\">\r\n"
                    "<input id=\"get\" type=\"submit\" value=\"Back to Menu\" />\r\n"
                    "</form>\r\n"
                    "</body>\r\n"
                    "</html>\r\n", name);
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
            
            return NULL;
            
                    
                    
        }else {
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
            "<br><br>\r\n"
            "<form method=\"POST\" enctype=\"text/plain\">\r\n"
            "<input id=\"get\" type=\"submit\" value=\"Cancel or Check Appointment\" />\r\n"
            "</form>\r\n"
            "</body>\r\n"
            "</html>\r\n";
            
            snprintf((char*)buff, sizeof(buff), "%s", webpage);
            
            write(connfd, (char*) buff, strlen((char *)buff));
            close(connfd);
        }
        return NULL;
        
    
    
    
}



