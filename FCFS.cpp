#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
using namespace std;

//These are find functions to locate a process in a vector
//This one value is the process
template<class InputIt, class T>
constexpr InputIt findP(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (first->name == value.name) {
            return first;
        }
    }
    return last;
}

//This one value is also the process
template<class InputIt, class T>
constexpr InputIt findP2(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (first->name == value->name) {
            return first;
        }
    }
    return last;
}

//This one value is the process' name
template<class InputIt, class T>
constexpr InputIt findP3(InputIt first, InputIt last, const T& value)
{
    for (; first != last; ++first) {
        if (first->name == value) {
            return first;
        }
    }
    return last;
}





struct Process{

    string name;
    int currBurst;
    int currIO;
    vector<int> bursts;
    vector<int> iotimes;
    int RT;
    int TT;
    int WT;
    int AT;
    bool hasRun;
    int lastBurstTime;
    int burstAndIOTotal = 0;



    Process(string name, vector<int> prcessdata){

        //Set up the bursts and iotimes vector
        int i = 0;
        for(int n: prcessdata){
            if(i++%2 == 0){
                bursts.push_back(n);
                burstAndIOTotal += n;
            }else{
                iotimes.push_back(n);
                burstAndIOTotal += n;
            }
        }
        
        //Initialize the name
        this->name = name;

        //initialize prcoess data
        RT = 0, WT = 0, TT = 0, AT = 0;
        hasRun = false;

        //set the first burst and IO to the first in each vector
        currBurst = bursts[0];
        currIO = iotimes[0];





        //get rid of the first io as it is now used
        iotimes.erase(iotimes.begin());
    }

};

struct FCFS{
    double cpuUTIL;
    int totalTime;
    int idleTime;
    vector<Process> waitingQueue;
    vector<Process> readyQueue;
    //vector<string> order;
    vector<Process> completed;

    //Process currPro;
    FCFS(){
        totalTime = 0;
        idleTime = 0;
    }

    bool isValid(){

        //make sure every Process in readyQueue is valid 
        for(Process p: readyQueue){
            if(p.currBurst != 0){
                return true;
            }
        }
        
        //if readyQueue is empty
        if(readyQueue.empty()){

            if(waitingQueue.empty()){
                return false;
            }
            vector<int> times;
            for(Process p: waitingQueue){
                times.push_back(p.AT);
            }

            sort(times.begin(), times.end());

            if(totalTime < times[0]){
                idleTime += times[0] - totalTime;
                totalTime = times[0];
                // order.push_back("Idle");
            }  
            return true;

        }

        return false;
    }

    void updateWaiting(int curr){

        for(Process& P: waitingQueue){
            P.currIO -= curr;
        }

        auto it = waitingQueue.begin();
        while(it != waitingQueue.end()){
            if(it->currIO <= 0){
                if(it->iotimes.size()){
                it->currIO = it->iotimes[0];
                it->iotimes.erase(it->iotimes.begin());
                }else{
                    it->currIO = 0;
                }
                readyQueue.push_back(*it);
                it = waitingQueue.erase(findP2(waitingQueue.begin(), waitingQueue.end(), it));
            }else{
                ++it;
            }
        }
    }

    Process& nextProcess(){
        vector<int> times;

        //If readyQueue is not empty
        if(!readyQueue.empty()){
            for(Process p: readyQueue){
                times.push_back(p.AT);
            }

            sort(times.begin(), times.end());

            if(totalTime < times[0]){
                idleTime = times[0] - totalTime;
                totalTime = times[0];
                // order.push_back("Idle");
            }
            
            if(readyQueue.size() > 1){
                if(readyQueue[0].AT != 0 && readyQueue[1].AT != 0){
                    if(readyQueue[0].AT == readyQueue[1].AT){
                        return readyQueue[1];
                    }
                }
            }

        for(int i = 0; i < readyQueue.size(); i++){
            if(readyQueue.at(i).AT == times[0]){
                return readyQueue.at(i);
            }
        }
            

            return readyQueue.at(0);
    }else{ //if ready queue is empty, go to waiting queue
        for(Process p: waitingQueue){
                times.push_back(p.AT);
            }

            sort(times.begin(), times.end());

            if(totalTime < times[0]){
                idleTime = times[0] - totalTime;
                totalTime = times[0];
                // order.push_back("Idle");
            }
            
            if(waitingQueue.size() > 1){
                if(waitingQueue[0].AT != 0 && waitingQueue[1].AT != 0){
                    if(waitingQueue[0].AT == waitingQueue[1].AT){
                        return waitingQueue[1];
                    }
                }
            }

        for(int i = 0; i < waitingQueue.size(); i++){
            if(waitingQueue.at(i).AT == times[0]){
                return waitingQueue.at(i);
            }
        }
            

            return waitingQueue.at(0);
    }
    }
};


int main(int argc, char const *argv[]){

    //Set up the processes
    Process P1("P1",{6, 21, 9, 28, 5, 26, 4, 22, 3, 41, 6, 45, 4, 27, 8 , 27, 3});
    Process P2("P2",{ 19, 48, 16, 32, 17, 29, 6, 44, 8, 34, 21, 34, 19, 39, 10, 31, 7 });
    Process P3("P3",{ 12, 14, 6, 21, 3, 29, 7, 45, 8, 54, 11, 44, 9 });
    Process P4("P4",{ 11, 45, 5, 41, 6, 45, 8, 51, 4, 61, 13, 54, 11, 61, 10 });
    Process P5("P5",{ 16, 22, 15, 21, 12, 31, 14, 26, 13, 31, 16, 18, 12, 21, 10, 33, 11 });
    Process P6("P6",{ 20, 31, 22, 30, 25, 29, 11, 44, 17, 34, 18, 31, 6, 22, 16 });
    Process P7("P7",{ 3, 44, 7, 24, 6, 34, 5, 54, 4, 24, 7, 44, 6, 54, 5, 21, 6, 43, 4 });
    Process P8("P8",{ 15, 50, 4, 23, 11, 31, 4, 31, 3, 47, 5, 21, 8, 31, 6, 44, 9});

    FCFS Processes;
    Processes.readyQueue = {P1,P2,P3,P4,P5,P6,P7,P8};

    while(Processes.isValid()){


        cout << "Current Time: " << Processes.totalTime << "\n" <<endl;


        Process& curr = Processes.nextProcess();

        cout << "Now running: " << curr.name << "\n" << endl;
        
        cout << "------------------------------------------------------------------------\n" << endl;

        cout << "Ready Queue:\tProcess\t\tBurst" << endl;

        if(Processes.readyQueue.size() < 2){
            cout << "\t\t[empty]" << endl;
        }
        for(Process P: Processes.readyQueue){
            if(P.name != curr.name){
                cout << "\t\t" << P.name << "\t" << P.currBurst << endl;
            }
        }

        cout << "\n------------------------------------------------------------------------\n" << endl;

        // Processes.order.push_back(curr.name);

        //get responseTime
        if(!curr.hasRun){
            curr.RT = Processes.totalTime;
        }

        Processes.totalTime += curr.currBurst;
        curr.AT = (Processes.totalTime + curr.currIO);
        int lastBurst = curr.currBurst;
        


        //update the process' burst
         if(curr.bursts.size() > 1){ //if there are 2 or more bursts
            curr.bursts.erase(curr.bursts.begin());
            curr.currBurst = curr.bursts[0];
            curr.hasRun = true;
        }else{ //if there is 1 or 0 bursts 
            curr.lastBurstTime = Processes.totalTime;

            //if the process is not already in the completed queue
            if(findP(Processes.completed.begin(), Processes.completed.end(), curr) == Processes.completed.end()) 
                Processes.completed.push_back(curr); 
        }

        cout << "Now in I/O:\tProcess\t\tRemaining I/O time" << endl;

        if(Processes.waitingQueue.size() < 1) {cout << "\t\t[empty]" << endl;}
        for(Process P: Processes.waitingQueue){
            if(P.name != curr.name){
                cout << "\t\t" << P.name << "\t" << P.currIO << endl;
            }
        }



        Processes.updateWaiting(lastBurst);
        
        //If the currect process isn't in completed queue
        if(findP(Processes.completed.begin(), Processes.completed.end(), curr) == Processes.completed.end()){
            Processes.waitingQueue.push_back(curr);
        }

        //cout << curr.name << " UNTIL - " << Processes.totalTime <<endl;

        //delete it from either readyQueue or waitingQueue depending on where it came from

        if(findP(Processes.readyQueue.begin(), Processes.readyQueue.end(), curr) == Processes.readyQueue.end()){
            Processes.waitingQueue.erase(findP(Processes.waitingQueue.begin(), Processes.waitingQueue.end(), curr));
        }else{
            Processes.readyQueue.erase(findP(Processes.readyQueue.begin(), Processes.readyQueue.end(), curr));
        }

        cout << "\n------------------------------------------------------------------------\n" << endl;

        cout << "Completed: \t";
        if(Processes.completed.empty()){cout << "[empty]\n";}
        for(Process P: Processes.completed){
            if(P.name != curr.name){ cout << P.name << "\t";}
        }

        cout << "\n------------------------------------------------------------------------" << endl;
        cout << "------------------------------------------------------------------------\n" << endl;

    }
    /*******************************************************************************************************************************************/
    //Here we are printing calculatiosn

    // sort(Processes.completed.begin(), Processes.completed.end(), compareProcess);

    cout << "Finished\n" << endl;
    cout << "Total Time:\t\t";
    cout << Processes.totalTime << endl;
    cout << "CPU Utilization:\t" << (static_cast<double>(Processes.totalTime-Processes.idleTime)/(Processes.totalTime))*100 << "%\n\n" << endl;
    
    //sort the processes by name (P1,P2,...)
    std::sort(Processes.completed.begin(), Processes.completed.end(),
        [](const Process &l, const Process &r) {
            return l.name < r.name;
    });

    //Print waiting times
    cout << "Waiting Times\t\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";
    double waitTotal = 0;
    for(Process P: Processes.completed){
        int value = ((P.lastBurstTime-P.RT) - P.burstAndIOTotal);
        waitTotal += (value);
        cout << value << "\t";
    }
    cout << "\nAverage Waiting:\t";
    cout << waitTotal/8 << "\n" << endl;


    //Print Turnaround Times
    cout << "Turnaround Times\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";
    double turnTotal = 0;
    for(Process P: Processes.completed){
        P.TT = (P.lastBurstTime-P.RT);
        turnTotal += (P.TT);
        cout << P.TT << "\t";
    }
    cout << "\nAverage Turnaround:\t";
    cout << turnTotal/8 << "\n" << endl;

    //Print Respone Times
    cout << "Response Times\t\tP1\tP2\tP3\tP4\tP5\tP6\tP7\tP8" << endl;
    cout << "\t\t\t";
    double responseTotal = 0;
    for(Process P: Processes.completed){
        responseTotal += P.RT;
        cout << P.RT << "\t";
    }
    cout << "\nAverage Response:\t";
    cout << responseTotal/8 << endl;

     return 0;
}
