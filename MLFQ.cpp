#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

#define TQ6 6
#define TQ12 12

//These are find functions to locate a process in a vector
template<class InputIt, class T>
constexpr InputIt findPIter(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (first->name == value->name) {
            return first;
        }
    }
    return last;
}

template<class InputIt, class T>
constexpr InputIt findPObject(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (first->name == value.name) {
            return first;
        }
    }
    return last;
}

struct Process {

    string name;
    int currBurst;      //the current burst
    int currIO;         //the current io
    vector<int> bursts; //vector ot brusts
    vector<int> iotimes;//vector of iotimes
    int RT;             //response time
    int TT;             //turnaround time
    int WT;             //waiting time
    int AT;             //arrival time
    bool hasRun;        //bool to check if process has run before
    int lastBurstTime;  //gets the last burst of the process    
    int burstAndIOTotal = 0;    //totals the burst and Io times for a process (for TT calc)
    int queue;                  //the queue that a process is in
                                //Queue 1 (RR with TQ 6)
                                //Queue 2 (RR with TQ 12)
                                //Queue 3 (FCFS)
    int state;                  //state of a process
                                //0 - ready
                                //1 - waiting
                                //2 - completed
    int queueTQ;
    int FCFSBurst;


    Process(string name, vector<int> prcessdata) {

        //Set up the bursts and iotimes vector
        int i = 0;
        for (int n : prcessdata) {
            if (i++ % 2 == 0) {             //for every other value in process data
                bursts.push_back(n);    //add value to bursts
                burstAndIOTotal += n;   //add value to total of bursts and ios
            }
            else {
                iotimes.push_back(n);   //or add value to iotimes
                burstAndIOTotal += n;   //add value to total of bursts and ios
            }
        }

        //Initialize the name
        this->name = name;

        //initialize queue to first queue
        queue = 1;

        //initialize state
        state = 0;

        //initialize queue TQ
        queueTQ = 6;

        //initialize prcoess data
        RT = 0, WT = 0, TT = 0, AT = 0;
        hasRun = false;

        //set the first burst and IO to the first in each vector
        currBurst = bursts[0];
        currIO = iotimes[0];

        //get rid of the first io and burst as they are now used
        iotimes.erase(iotimes.begin());
        bursts.erase(bursts.begin());
    }
};

struct MLFQ {
    double cpuUTIL;                 //CPU utilization
    int totalTime;                  //total time (current time)
    int idleTime;                   //count of how much the processes were idle
    vector<Process> waitingQueue;   //waiting queue for processes
    vector<Process> readyQueue;     //ready queue for processes
    vector<Process> completed;      //completed queue for processes
    vector<Process> smallQueue;     //vector to hold the prcoesses of the smallest queue

    MLFQ() {
        totalTime = 0;  //set total time to 0
        idleTime = 0;   //set idle time to 0
    }

    bool isValid() {

        //check the ready queue
        for (Process p : readyQueue) {
            if (p.currBurst != 0) { //if there exist such a process where the currBurst != 0
                return true; //return true
            }
        }

        //if the readyQueue is empty...
        if (readyQueue.empty()) {

            //and waiting queue is empty...
            if (waitingQueue.empty()) {
                return false; //return false as nothing can be done
            }

            //if waiting queue is not empty...
            //we are going to make sure the time/idle situation is accounted for
            //then return true

            //sort the waiting Queue by arrival time
            std::sort(waitingQueue.begin(), waitingQueue.end(),
                [](const Process& l, const Process& r) {
                    return l.AT < r.AT;
                });

            //if the totalTime is less than the time in the waitingQueue
            if (totalTime < waitingQueue[0].AT) {
                int ttime = waitingQueue[0].AT; //store lowest arrival time
                idleTime += (waitingQueue[0].AT - totalTime); //add the time diff to idletime
                updateWaiting(waitingQueue[0].AT - totalTime);

                totalTime = ttime;           //set total time to lowest arrival time
            }

            return true;
        }

        return false;
    }

    void updateWaiting(int curr) {

        //for everyprocess in the waiting queue 
        for (Process& P : waitingQueue) {
            P.currIO -= curr; //reduce the IO times by the curr IO burst
        }

        //go through the waiting queue
        auto it = waitingQueue.begin();
        while (it != waitingQueue.end()) {

            if (it->currIO <= 0) { //if the process completed IO

                if (it->iotimes.size()) { //if io vector for process is not empty
                    it->currIO = it->iotimes[0]; //currIO is the first io 
                    it->iotimes.erase(it->iotimes.begin()); //erase the io as you just updated io
                }
                else {  //if io vector is empty
                    it->currIO = -1;
                }

                if (it->bursts.empty() && it->iotimes.empty() && it->currBurst <= 0) { //if bursts and io of process is empty
                    it->state = 2;
                    completed.push_back(*it);
                }
                else { //if it can go again
                    it->state = 0;
                    readyQueue.push_back(*it); //push back the process to ready queue
                }

                it = waitingQueue.erase(findPIter(waitingQueue.begin(), waitingQueue.end(), it)); //erase it from waiting queue
            }
            else { //if the process is not valid to be put back (not done with IO) 
                ++it; //increment the iterator
            }
        }
    }


    Process& nextProcess() {


        while (readyQueue.empty()) { //while ready queue is empty
            updateWaiting(1); //update waiting queue by 1
            idleTime++; //increase idle time;
        }

        //now ready queue is not empty...

        //sort the readyQueue by queue
        sort(readyQueue.begin(), readyQueue.end(),
            [](const Process& l, Process& r) {
                return l.queue < r.queue;
            });

        //make a vector to hold the processes of the smallest Queue
        smallQueue.clear();

        //Find all the processes that are of the smallest queue 
        //and add them to array
        for (Process& p : readyQueue) {
            if (p.queue == readyQueue[0].queue) {
                smallQueue.push_back(p);
            }
        }

        //sort smallQueue by arrival time
        sort(smallQueue.begin(), smallQueue.end(),
            [](const Process& l, const Process& r) {
                return l.AT < r.AT;
            });


        //if the totalTime is less than the
        //smallest arrival time of the smallqueue
        if (totalTime < smallQueue[0].AT) {
            idleTime += (smallQueue[0].AT - totalTime);     //add the time diff to idletime
            updateWaiting(smallQueue[0].AT - totalTime);
            totalTime = smallQueue[0].AT;                   //set total time to lowest arrival time
        }

        //return the process with the smallest 
        //arrival time from the smallqueue
        return smallQueue.at(0);
    }


    void runProcess(Process& curr) {


        int queue = curr.queue;



        if (queue == 1) { //if process is in first queue (RR TQ 6)

            if (curr.currBurst > TQ6) { //if the current burst for the process is more than TQ(6)
                curr.currBurst -= TQ6;  //decrement the current burst by the TQ
                curr.queue++;           //chnage the queue as the burst was not completed
                curr.queueTQ = 12;      //chnage queue TQ to 12 as it is now Q2
                setResponseTime(curr);  //set response time
                totalTime += TQ6;       //chnage the totalTime
                curr.AT = totalTime;       //arival time is set to the current total time
                updateWaiting(TQ6);        //update waitign queue


                //update process in readyQueue
                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                //check what queue it should be pushed back to
                if (curr.AT > totalTime) {
                    curr.state = 1;
                    waitingQueue.push_back(curr);
                }
                else if (curr.AT <= totalTime) {
                    curr.state = 0;
                    readyQueue.push_back(curr);
                }


            }
            else if (curr.currBurst <= TQ6) { //if the burst is smaller than the TQ
                totalTime += curr.currBurst;    //incrememnt the total time by the burst
                setResponseTime(curr);          //set response time
                updateWaiting(curr.currBurst); //update the waiting queue
                curr.AT = totalTime + curr.currIO;


                if (!curr.bursts.empty()) { //if the process has more bursts
                    curr.currBurst = *curr.bursts.begin(); //assign the process a new burst
                    curr.bursts.erase(curr.bursts.begin()); //erase the val as you just used it
                }
                else { //there are no more bursts

                    if (curr.currBurst > 0) {
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                        curr.state = 0;
                        readyQueue.push_back(curr);
                        return;
                    }
                    else {
                        curr.lastBurstTime = totalTime;
                        curr.state = 2; //change state to compeleted
                        completed.push_back(curr); //push to completed as there is no IO and bursts

                        //erase if from the ready queue
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                        return; //exit, nothing more to do
                    }
                }

                //add process to waiting queue and update state
                curr.state = 1;
                waitingQueue.push_back(curr);



                //remove it from the ready queue
                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

            }
        }
        else if (queue == 2) { //if process is in second queue (RR TQ 12)

            curr.queueTQ = 12;

            while (canStillRun(curr)) {
                curr.currBurst--; //run the process for one burst
                curr.queueTQ--; //decrement the curr queue TQ
                totalTime++; //increment the totaltime as the process has ran
                curr.AT = totalTime;
                updateWaiting(1); //update the waiting


                if (curr.queueTQ <= 0) { //if there is not any more of time quantum left
                    if (curr.currBurst <= 0) { //and the burst has finished as well

                        if (!curr.bursts.empty()) { //if the process has more bursts
                            curr.currBurst = *curr.bursts.begin(); //assign the process a new burst
                            curr.bursts.erase(curr.bursts.begin()); //erase the val as you just used it
                        }
                        else { //there are no more bursts

                            if (curr.currBurst > 0) { //but there is still burst to complete, put it back in RQ
                                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                                curr.state = 0;
                                readyQueue.push_back(curr);
                                return;
                            }
                            else { //send it to complete
                                curr.lastBurstTime = totalTime;
                                curr.state = 2; //change state to compeleted
                                completed.push_back(curr); //push to completed as there is no IO and bursts

                                //erase if from the ready queue
                                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                                return; //exit, nothing more to do
                            }
                        }

                        //erase it from the ready queue
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                        //add it to waiting queue
                        curr.state = 1;
                        curr.AT = totalTime + curr.currIO;
                        waitingQueue.push_back(curr);



                        return; //exit, nothing more to do


                    }
                    else { //there is burst remaining but we have no more TQ

                       //move process to next queue (FCFS) and exit the function as it cannot run anymore

                        curr.queue++;

                        //check AT and see where it should go
                        if (curr.AT > totalTime) {
                            curr.state = 1;
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                            waitingQueue.push_back(curr);
                        }
                        else if (curr.AT <= totalTime) {
                            curr.state = 0;
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                            readyQueue.push_back(curr);
                        }


                        return;
                    }
                }
                else if (curr.currBurst == 0) { //also check if the currBurst is 0 independently

                    curr.AT = totalTime + curr.currIO;

                    //update the bursts to the next one
                    if (!curr.bursts.empty()) { //if the process has more bursts
                        curr.currBurst = *curr.bursts.begin(); //assign the process a new burst
                        curr.bursts.erase(curr.bursts.begin()); //erase the val as you just used it
                    }
                    else { //if there are no more bursts

                        if (curr.currBurst > 0) { //but there is still burst to complete, put it back in RQ
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                            curr.state = 0;
                            readyQueue.push_back(curr);
                            return;
                        }
                        else { //send it to complete
                            curr.lastBurstTime = totalTime;
                            curr.state = 2; //change state to compeleted
                            completed.push_back(curr); //push to completed as there is no IO and bursts

                            //erase if from the ready queue
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                            return; //exit, nothing more to do
                        }
                    }

                    //check AT and see where it should go
                    if (curr.AT > totalTime) {
                        curr.state = 1;
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                        waitingQueue.push_back(curr);
                    }
                    else if (curr.AT <= totalTime) {
                        curr.state = 0;
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                        readyQueue.push_back(curr);
                    }


                    return; //exit, nothing more to do

                }
                //if we get down here then the process did not finish / got preempted 
                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                readyQueue.push_back(curr);
            }

        }
        else if (curr.queue == 3) { //if process is in queue 3 (FCFS)

            curr.FCFSBurst = curr.currBurst;

            while (canStillRun(curr)) {
                curr.currBurst--; //run the process for one burst
                curr.FCFSBurst--; //decrement the curr FCFS burst
                totalTime++; //increment the totaltime as the process has ran
                curr.AT = totalTime;       //set the arrivaltime
                updateWaiting(1); //update the waiting


                if (curr.currBurst == 0) { //if the process has finished its burst
                    curr.AT = totalTime + curr.currIO;

                    if (!curr.bursts.empty()) { //if the process has more bursts
                        curr.currBurst = *curr.bursts.begin(); //assign the process a new burst
                        curr.bursts.erase(curr.bursts.begin()); //erase the val as you just used it
                    }
                    else { //there are no more bursts

                        if (curr.currBurst != 0) { //but there is still burst to complete, put it back in RQ
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                            curr.state = 0;
                            readyQueue.push_back(curr);
                            return;
                        }
                        else { //send it to complete
                            curr.lastBurstTime = totalTime;
                            curr.state = 2; //change state to compeleted
                            completed.push_back(curr); //push to completed as there is no IO and bursts

                            //erase if from the ready queue
                            readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));

                            return; //exit, nothing more to do
                        }
                    }
                    //if there are more bursts

                    //check AT and see where it should go
                    if (curr.AT > totalTime) {
                        curr.state = 1;
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                        waitingQueue.push_back(curr);
                    }
                    else if (curr.AT <= totalTime) {
                        curr.state = 0;
                        readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                        readyQueue.push_back(curr);
                    }


                    return; //exit, nothing more to do


                }

                //if we get down here then the process did not finish / got preempted 
                readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
                readyQueue.push_back(curr);
            }

            // //if we get down here then the process did not finish / got preempted 
            // readyQueue.erase(findPObject(readyQueue.begin(), readyQueue.end(), curr));
            // readyQueue.push_back(curr);


        }
    }

    bool canStillRun(Process& curr) {
        for (Process p : readyQueue) { //if a process shows in up ready qeueue
            if (p.queue < curr.queue) { //with a higher priority queue
                if (curr.queue < 3) { //if the process'queue is not 3 (lowest priority)
                    curr.queue++;       //increment prcoess' queue, it just preempted
                } //other we will keep the queue the same
                return false; //return false because it can be preempted
            }
        }

        return true; //otherwise return true
    }

    void setResponseTime(Process& curr) {

        if (!curr.hasRun) { //if the process has not run
            curr.RT = totalTime; //response time is totaltime
            curr.hasRun = true; //make that process has run
        }
    }


    void setArrivalTime(Process& curr) {

        curr.AT = totalTime + curr.currIO;
    }
};

int main(int argc, char const* argv[]) {

    //Set up the processes with process data (CPU,IO,CPU,IO,....)
    Process P1("P1", { 6, 21, 9, 28, 5, 26, 4, 22, 3, 41, 6, 45, 4, 27, 8 , 27, 3 });
    Process P2("P2", { 19, 48, 16, 32, 17, 29, 6, 44, 8, 34, 21, 34, 19, 39, 10, 31, 7 });
    Process P3("P3", { 12, 14, 6, 21, 3, 29, 7, 45, 8, 54, 11, 44, 9 });
    Process P4("P4", { 11, 45, 5, 41, 6, 45, 8, 51, 4, 61, 13, 54, 11, 61, 10 });
    Process P5("P5", { 16, 22, 15, 21, 12, 31, 14, 26, 13, 31, 16, 18, 12, 21, 10, 33, 11 });
    Process P6("P6", { 20, 31, 22, 30, 25, 29, 11, 44, 17, 34, 18, 31, 6, 22, 16 });
    Process P7("P7", { 3, 44, 7, 24, 6, 34, 5, 54, 4, 24, 7, 44, 6, 54, 5, 21, 6, 43, 4 });
    Process P8("P8", { 15, 50, 4, 23, 11, 31, 4, 31, 3, 47, 5, 21, 8, 31, 6, 44, 9 });

    MLFQ Processes;

    //Setup the readyQueue with processes.
    Processes.readyQueue = { P1,P2,P3,P4,P5,P6,P7,P8 };

    while (Processes.isValid()) { //while there is another process that could run

        cout << "Current Time: " << Processes.totalTime << "\n" << endl;

        Process& curr = Processes.nextProcess();    //select the next process



        //Vector copies to not mess with calculations 
        vector<Process> waitingQueue = Processes.waitingQueue;
        vector<Process> readyQueue = Processes.readyQueue;
        vector<Process> completed = Processes.completed;

        cout << "Now running: " << curr.name << "\n" << endl;
        cout << "\n------------------------------------------------------------------------\n" << endl;


        //sort the processes by name (P1,P2,...)
        std::sort(readyQueue.begin(), readyQueue.end(),
            [](const Process& l, const Process& r) {
                return l.name < r.name;
            });

        //sort the processes by name (P1,P2,...)
        std::sort(readyQueue.begin(), readyQueue.end(),
            [](const Process& l, const Process& r) {
                return l.name < r.name;
            });

        //sort the processes by name (P1,P2,...)
        std::sort(completed.begin(), completed.end(),
            [](const Process& l, const Process& r) {
                return l.name < r.name;
            });


        cout << "Ready Queue:\tProcess\t\tBurst" << endl;
        if (readyQueue.empty()) {
            cout << "\t\t[empty]" << endl;
        }
        for (Process P : readyQueue) {
            cout << "\t\t" << P.name << "\t\t" << P.currBurst << endl;
        }

        cout << "\n------------------------------------------------------------------------\n" << endl;

        cout << "Now in I/O:\tProcess\t\tRemaining I/O time" << endl;

        if (waitingQueue.empty()) { cout << "\t\t[empty]" << endl; }
        for (Process P : waitingQueue) {
            cout << "\t\t" << P.name << "\t\t" << P.currIO << endl;
        }

        cout << "\n------------------------------------------------------------------------\n" << endl;

        if (!completed.empty()) {
            cout << "Completed: \t";
            for (Process P : completed) {
                cout << P.name << "\t";
            }
            cout << "\n" << endl;
        }


        Processes.runProcess(curr);                 //run that process


        cout << "\n------------------------------------------------------------------------\n" << endl;
        cout << "------------------------------------------------------------------------\n" << endl;
    }



    cout << "Finished\n" << endl;
    cout << "Total Time:\t\t";
    cout << Processes.totalTime << endl;
    cout << "CPU Utilization:\t" << (static_cast<double>(Processes.totalTime - Processes.idleTime) / (Processes.totalTime)) * 100 << "%\n\n" << endl;

    //sort the processes by name (P1,P2,...)
    std::sort(Processes.completed.begin(), Processes.completed.end(),
        [](const Process& l, const Process& r) {
            return l.name < r.name;
        });

    //Print waiting times
    cout << "Waiting Times\t\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";

    double waitTotal = 0;
    for (Process P : Processes.completed) {
        int value = (P.lastBurstTime - P.burstAndIOTotal);
        waitTotal += (value);
        cout << value << "\t";
    }
    cout << "\nAverage Waiting:\t";
    cout << waitTotal / 8 << "\n" << endl;

    //Print Turnaround Times
    cout << "Turnaround Times\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";
    double turnTotal = 0;
    for (Process& P : Processes.completed) {
        P.TT = P.lastBurstTime;
        turnTotal += (P.TT);
        cout << P.lastBurstTime << "\t";
    }
    cout << "\nAverage Turnaround:\t";
    cout << turnTotal / 8 << "\n" << endl;

    //Print Respone Times
    cout << "Response Times\t\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";
    double responseTotal = 0;
    for (Process P : Processes.completed) {
        responseTotal += P.RT;
        cout << P.RT << "\t";
    }
    cout << "\nAverage Response:\t";
    cout << responseTotal / 8 << endl;

    return 0;
}
