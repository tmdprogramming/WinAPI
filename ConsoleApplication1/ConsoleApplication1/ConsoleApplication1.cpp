#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <tlhelp32.h>
#include <locale>
#include <codecvt>
#include <fstream>
#include <io.h>
#include <fcntl.h>
#include <string>
#define THREADCOUNT 5
#define MAX_SIZE 2048
#define SIZE 2048
using namespace std;


class Queue {
private:
    int items[SIZE], front, rear;

public:
    Queue() {
        front = -1;
        rear = -1;
    }

    bool isFull() {
        if (front == 0 && rear == SIZE - 1) {
            return true;
        }
        return false;
    }

    bool isEmpty() {
        if (front == -1)
            return true;
        else
            return false;
    }

    void enQueue(int element) {
        if (isFull()) {
            cout << "Queue is full";
        }
        else {
            if (front == -1) front = 0;
            rear++;
            items[rear] = element;
            cout << endl
                << "Inserted " << element << endl;
        }
    }

    int deQueue() {
        int element;
        if (isEmpty()) {
            cout << "Queue is empty" << endl;
            return (-1);
        }
        else {
            element = items[front];
            if (front >= rear) {
                front = -1;
                rear = -1;
            } /* Q has only one element, so we reset the queue after deleting it. */
            else {
                front++;
            }
            cout << endl
                << "Deleted -> " << element << endl;
            return (element);
        }
    }

    void display() {
        /* Function to display elements of Queue */
        int i;
        if (isEmpty()) {
            cout << endl
                << "Empty Queue" << endl;
        }
        else {
            cout << endl
                << "Front index-> " << front;
            cout << endl
                << "Items -> ";
            for (i = front; i <= rear; i++)
                cout << items[i] << "  ";
            cout << endl
                << "Rear index-> " << rear << endl;
        }
    }
};
Queue q;
bool wrote_to_file = false;
HANDLE ghMutex;
HANDLE snapShot = INVALID_HANDLE_VALUE;
PROCESSENTRY32 ProcessInfo = { 0 };
wofstream f(L"sample.txt");
DWORD WINAPI WriteToDatabase(LPVOID lpParam);
int main() {
    //needed to convert wide char string for writing to file
    const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>());
    f.imbue(utf8_locale);
    //num is input
    int num;
    //choice is the number of pid's the user wants to check
    string choice;
    //define the dwsize
    ProcessInfo.dwSize = sizeof(PROCESSENTRY32);
    //get user to give us ints to check
        
        for (int x = 0; x < THREADCOUNT; x++)
        {
            cout << "enter "<< THREADCOUNT - x << " ints to check (To change, change THREADCOUNT)" << endl;
            cin >> num;
            q.enQueue(num);
            if (x == THREADCOUNT - 1)
            {
                cout << "here is your queue: \n" << endl;
                q.display();
            }
        }
        //declare thread array
        HANDLE aThread[THREADCOUNT];
        DWORD ThreadID;
        int i;
        // Create a mutex with no initial owner
        ghMutex = CreateMutex(
            NULL,              // default security attributes
            FALSE,             // initially not owned
            NULL);             // unnamed mutex
        if (ghMutex == NULL)
        {
            printf("CreateMutex error: %d\n", GetLastError());
            return 1;
        }
        // Create worker threads
        for (i = 0; i < THREADCOUNT; i++)
        {
            aThread[i] = CreateThread(
                NULL,       // default security attributes
                0,          // default stack size
                (LPTHREAD_START_ROUTINE)WriteToDatabase,
                NULL,       // no thread function arguments
                0,          // default creation flags
                &ThreadID); // receive thread identifier

            if (aThread[i] == NULL)
            {
                printf("CreateThread error: %d\n", GetLastError());
                return 1;
            }
        }
        // Wait for all threads to terminate
        WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);
        // Close all handles and file
        for (i = 0; i < THREADCOUNT; i++)
        {
            CloseHandle(aThread[i]);
        }
        CloseHandle(ghMutex);
        CloseHandle(snapShot);
        f.close();
        return 0;
    }

DWORD WINAPI WriteToDatabase(LPVOID lpParam)
{
   
    DWORD dwWaitResult;
    
    // Request ownership of mutex.

        dwWaitResult = WaitForSingleObject(
            ghMutex,    // handle to mutex
            INFINITE);  // no time-out interval
        int this_thread_int = q.deQueue();
         if (this_thread_int != -1)
        {
             printf("Thread %d is checking PID: "  ,
                 GetCurrentThreadId());
             cout << this_thread_int << endl;
                snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                if (INVALID_HANDLE_VALUE == snapShot)
                {
                    cout << "Snap shot Failed with error: " << GetLastError() << endl;
                }
                while (Process32Next(snapShot, &ProcessInfo) != FALSE)
                {
                    if (this_thread_int == ProcessInfo.th32ProcessID)
                    {
                        
                        
                        f << "PID " << ProcessInfo.th32ProcessID << ": " << ProcessInfo.szExeFile << endl;
                        cout << "Process name: ";
                        wcout << ProcessInfo.szExeFile << endl;
                        cout << "\t PID = " << ProcessInfo.th32ProcessID << endl;
                        wrote_to_file = true;
                    }
                    //else
                    //{
                    //    f << "PID = " << this_thread_int << "is invalid/not included on snap shot" << endl;
                    // }
                }
                if (wrote_to_file == false)
                {
                    f << "PID = " << this_thread_int << " is invalid/not included on snap shot" << endl;
                    cout << "PID = " << this_thread_int << " is invalid/not included on snap shot" << endl;
                }
        }
         wrote_to_file = false;
                return 0;
}

  
