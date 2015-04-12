#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <string>
#include <deque>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>


using namespace std;

#include "noncanmode.h"


deque <char*> history(10, NULL);
int historySize = 0;

void printDir(int pwd)
{
	char curDir[50];
	getcwd(curDir, 50);
	if (strlen(curDir) >  16 && pwd == 0)
	{
		string temp;
		int i;
		for (i = strlen(curDir) - 1; curDir[i] != '/'; i--);
		string curDirStr = string(curDir);
		temp =  curDirStr.substr(i, strlen(curDir));
		strcpy(curDir, temp.c_str());
		//curDir[temp.size()] = '\n';
		write(STDOUT_FILENO, "/...", 4);
	}

	write(STDOUT_FILENO, curDir, strlen(curDir));
	
    if (pwd)
        write(STDOUT_FILENO, "\n", 1);
    else
    {
        write(STDOUT_FILENO, ">", 1);
        write(STDOUT_FILENO, " ", 1);
    }
        
}

void writePermissions( mode_t mode )
{
    if (S_ISDIR(mode)) write (STDOUT_FILENO, "d", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IRUSR) == S_IRUSR) write(STDOUT_FILENO, "r", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IWUSR) == S_IWUSR) write(STDOUT_FILENO, "w", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IXUSR) == S_IXUSR) write(STDOUT_FILENO, "x", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IRGRP) == S_IRGRP) write(STDOUT_FILENO, "r", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IWGRP) == S_IWGRP) write(STDOUT_FILENO, "w", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IXGRP) == S_IXGRP) write(STDOUT_FILENO, "x", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IROTH) == S_IROTH) write(STDOUT_FILENO, "r", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IWOTH) == S_IWOTH) write(STDOUT_FILENO, "w", 1);
    else write(STDOUT_FILENO, "-", 1);
    if ((mode & S_IXOTH) == S_IXOTH) write(STDOUT_FILENO, "x", 1);
    else write(STDOUT_FILENO, "-", 1);
    
}

void ls()
{
    DIR *currentDir;
    struct dirent *file;
    struct stat fileStat;
    
    currentDir = opendir(".");
    if (currentDir)
    {
        file = readdir(currentDir);
        while ( file != NULL )
        {

            if ( stat( file->d_name, &fileStat ) == 0)
            {
                writePermissions (fileStat.st_mode);
            }
            else cout << "stat fail";
            
            write(STDOUT_FILENO, " ", 1);
            write(STDOUT_FILENO, file->d_name, strlen(file->d_name) );
            write(STDOUT_FILENO, "\n", 1);
            file = readdir(currentDir);
        }
    }
    else cout << "opendir fail";
    
}


void printHistory()
{
    
    for (int i=0; i < historySize; i++)
    {
        char num = i + '0';
        write(STDOUT_FILENO, &num, 1);
        write(STDOUT_FILENO, " ", 1);
        write(STDOUT_FILENO, history[historySize -1-i], strlen(history[historySize -1-i]) );
        write(STDOUT_FILENO, "\n", 1);
    }
    
    //write(STDOUT_FILENO, "\n", 2);
}


void readCan()
{
	struct termios SavedTermAttributes;
	    char RXChar;
    string buffer;
	    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    int pos =-1;
    
	    while(1){
	        read(STDIN_FILENO, &RXChar, 1);
	        if(0x0A == RXChar){ // C-d
                char *toBeAdded = new char[buffer.length()+1];
                strcpy( toBeAdded, buffer.c_str());
                
                if (buffer.length() > 0)
                {
                    if (historySize >= 10) history.pop_back();
                    else
                    {
                        historySize++;
                    }
                    
                        history.push_front( toBeAdded );
                    break;
                }
                write(STDOUT_FILENO, "\n", 1);
                printDir(0);
                
	        }
            else if ( ((0x7F == RXChar) || (0x08 == RXChar)) && (buffer.length() > 0) ) // Backspace
            {
                char backspace = 8;
                write(STDOUT_FILENO, &backspace, 1);
                write(STDOUT_FILENO, " ", 1);
                write(STDOUT_FILENO, &backspace, 1);
                buffer.erase(buffer.length()-1, buffer.length());
                
            } // End Backspace
	        else{
                if (RXChar == '[') // Arrow Up/Down
                {
                    read(STDIN_FILENO, &RXChar, 1);
                    
                    if (RXChar == 65) // Up Arrow
                    {
                        pos++;
                        if (pos >= historySize)
                        {
                            pos--;
                            char bell = 7;
                            
                            write(STDOUT_FILENO, &bell , 1);
                        }
                        else
                        {
                            //cout<< "\nbuffer = "<<buffer<<endl;
                            for (int i=0; i < buffer.length(); i++)
                            {
                                char backspace = 8;
                                write(STDOUT_FILENO, &backspace, 1);
                            }
                            for (int i=0; i < buffer.length(); i++)
                            {
                                write(STDOUT_FILENO, " ", 1);
                            }
                            for (int i=0; i < buffer.length(); i++)
                            {
                                char backspace = 8;
                                write(STDOUT_FILENO, &backspace, 1);
                            }
                            
                            buffer = history[pos];
                            char *temp = new char[buffer.length()+1];
                            strcpy(temp, buffer.c_str());
                        
                            write(STDOUT_FILENO, temp, buffer.length() + 1);
                        }
                        
                        
                        
                    }
                    else if (RXChar == 66) // Down Arrow
                    {
                        pos--;
                        if (pos < 0)
                        {
                            pos++;
                            char bell = 7;
                            
                            write(STDOUT_FILENO, &bell , 1);
                        }
                        else
                        {
                            for (int i=0; i < buffer.length(); i++)
                            {
                                char backspace = 8;
                                write(STDOUT_FILENO, &backspace, 1);
                            }
                            for (int i=0; i < buffer.length(); i++)
                            {
                                write(STDOUT_FILENO, " ", 1);
                            }
                            for (int i=0; i < buffer.length(); i++)
                            {
                                char backspace = 8;
                                write(STDOUT_FILENO, &backspace, 1);
                            }
                            
                            
                            buffer = history[pos];
                            char *temp = new char[buffer.length()+1];
                            strcpy(temp, buffer.c_str());
                            
                            write(STDOUT_FILENO, temp, buffer.length() + 1);
                        }
                        
                    }
                } // End Arrow Up/Down
	            else if(isprint(RXChar)){
                    buffer += RXChar;
                    char* temp = &RXChar;
                    write(STDOUT_FILENO, temp, 1);
                    
	            }
	            else{
	                //printf("RX: ' ' 0x%02X\n",RXChar);
	            }
	        }
	    }
	    
	    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);	
}

void regularRun(string args[], int indexofBegin, int indexofEnd )
{
    int *childStatus = new int;
    // string array to Char array
    char *argsChar[indexofEnd+1];
    for (int i=indexofBegin; i<indexofEnd; i++)
    {
        argsChar[i] = new char[args[i].length()];
        strcpy( argsChar[i], args[i].c_str() );
       
    }
    argsChar[indexofEnd] = NULL;

    // FORK
    int cpid = fork();
    //int status;
    if (cpid == 0) // Child
    {
        int returnValue = execvp(argsChar[0], argsChar);
        //exit(0);
    }
    else
    {
        //int status;
        wait(childStatus);
        //write(STDOUT_FILENO, "\n", 1);
        //cout << "Done" << endl;
    }

}

void runWithPiping(string args[], int indexofBegin, int indexofEnd, int in, int out, int pipe[])
{
    int childStatus;

    // string array to Char array
    char *argsChar[indexofEnd+1];
    for (int i=indexofBegin; i<=indexofEnd; i++)
    {
        argsChar[i] = new char[args[i].length()];
        strcpy( argsChar[i], args[i].c_str() );
       
    }
    argsChar[indexofEnd+1] = NULL;

    // FORK
    int cpid = fork();
    //int status;
    if (cpid == 0) // Child
    {
        if (out == 1)
        {
            cout << " Writing into pipe!" << endl;
            dup2(pipe[1], STDOUT_FILENO);
        }
        if (in == 1)
        {
            cout << " Reading from pipe!" << endl;
            dup2(pipe[0], STDIN_FILENO);
        }

        int returnValue = execvp(argsChar[0], argsChar);
        exit(0);
    }
    else
    {
        //int status;
        cout << "Waiting..." << endl;
        //wait(&childStatus);
        //write(STDOUT_FILENO, "\n", 1);
        //cout << "Done" << endl;
    }

}


void elseStuff()
{
    string buffer = string(history[0]);
    string args[20];
    int argsIndex = 0;
    int begin = 0;

    int pipeIndex = 0;
    for (int i=0; i < buffer.length(); i++)
    {
        if (buffer[i] == ' ' )
        {
            if (begin != i)
            {
                args[argsIndex] = buffer.substr(begin, i-begin);
                argsIndex++;
            }
            begin = i + 1;

        }
        else if ( buffer[i] == '|' )
        {
            args[argsIndex] = "|";
            argsIndex++;
            begin = i + 1;

        }
        else if (buffer[i] == '<' )
        {
            
        }
        else if (buffer[i] == '>')
        {
            
        }
        else if (i == (buffer.length() -1))
        {
            args[argsIndex] = buffer.substr(begin, i-begin+1);
            argsIndex++;
        }
    } // Argument List has now been loaded into args[], an array of strings

    int refToPipe[3];
    pipe (refToPipe);
    //runWithPiping(args, 0, 1, 0, 1, refToPipe );
    //runWithPiping(args, 3, 3, 1, 0, refToPipe );
    //int childStatus;
    //wait(&childStatus);




    int *childStatus = new int;

    // string array to Char array
    char *argsChar[3+1];
    for (int i=0; i<=1; i++)
    {
        argsChar[i] = new char[args[i].length()];
        strcpy( argsChar[i], args[i].c_str() );
       
    }
    argsChar[1+1] = NULL;

    // FORK
    int catPid = fork();
    int grepPid = fork();
    //int status;
    if (catPid == 0) // Child
    {
        
        dup2(refToPipe[1], STDOUT_FILENO);
        int returnValue = execvp(argsChar[0], argsChar);
        exit(0);
    }
    else if (grepPid == 0)
    {
        for (int i=0, j = 3; i<=1; i++, j++)
        {
            argsChar[i] = new char[args[i].length()];
            strcpy( argsChar[i], args[j].c_str() );
        }
        argsChar[1+1] = NULL;
        dup2(refToPipe[0], STDIN_FILENO);
        int returnValue = execvp(argsChar[0], argsChar);
    }
    else
    {
        //int status;
        cout << "Waiting..." << endl;
        wait(childStatus);
        //write(STDOUT_FILENO, "\n", 1);
        //cout << "Done" << endl;
    }


}

int main()
{
	do
	{
		printDir(0);
		readCan();
        write(STDOUT_FILENO, "\n", 1);
        
        // Process first thing in buffer
        
        if ( strcmp( history[0], "history") == 0)
        {
            printHistory();
        }
        else if (strcmp(history[0], "pwd") == 0)
        {
            printDir(1);
        }
        else if (history[0][0] == 'c' && history[0][1] == 'd')
        {
            if (history[0][2] == ' ')
            {
                int i;
                for (i = 3; i < strlen(history[0]) && history[0][i] == ' '; i++);
                //i++;
                if (chdir( &(history[0][i]) ) == -1 )
                {
                    write(STDOUT_FILENO, "Error changing directory.\n", 27);
                }
            }
            else
            {
                chdir( getenv("HOME") );
            }
        }
        else if ( strcmp(history[0], "exit") ==0)
        {
            return 0;
        }
        else if ( strcmp(history[0], "ls") ==0)
        {
            ls();
        }
        else
        {
            elseStuff();
        }
        
       // cout << endl << "History: " << history[0] << endl;

		//read(STDIN_FILENO, buff, 128);
		//cout << "break" <<endl;
		
		//break;
	}while(1);
	
	return 0;
}