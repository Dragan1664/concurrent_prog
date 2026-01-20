#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <mutex>
#include <algorithm>
#include <thread>

class OrganTransplant {

private:
    std::vector<std::string> waitingList;
    // date is expected in format: dd-mm-yyyy
    std::vector<std::tuple<std::string, std::string>> treatedList;
    std::mutex mtx;

public:

    bool AddToWaitingList(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mtx);

        waitingList.push_back(name);
        return true;
    }

    bool MoveFromWaitingToTreated(const std::string& name,
                                  const std::string& date)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto it = std::find(waitingList.begin(),
                            waitingList.end(),
                            name);

        if (it == waitingList.end())
            return false;

        waitingList.erase(it);
        treatedList.push_back({name, date});

        return true;
    }

void RemoveTreatedBefore(const std::string& date)
{
    std::lock_guard<std::mutex> lock(mtx);

    int refDay   = std::stoi(date.substr(0,2));
    int refMonth = std::stoi(date.substr(3,2));
    int refYear  = std::stoi(date.substr(6,4));

    for (auto it = treatedList.begin(); it != treatedList.end(); )
    {
        std::string d = std::get<1>(*it);

        int day   = std::stoi(d.substr(0,2));
        int month = std::stoi(d.substr(3,2));
        int year  = std::stoi(d.substr(6,4));

        bool remove = false;

        if (year < refYear)
            remove = true;

        else if (year == refYear && month < refMonth)
            remove = true;

        else if (year == refYear &&
                 month == refMonth &&
                 day < refDay)
            remove = true;

        if (remove)
            it = treatedList.erase(it);
        else
            ++it;
    }
}


    void PrintWaiting()
    {
        std::lock_guard<std::mutex> lock(mtx);

        std::cout << "WAITING LIST\n";
        for (auto& p : waitingList)
            std::cout << p << std::endl;
    }

    void PrintTreated()
    {
        std::lock_guard<std::mutex> lock(mtx);

        std::cout << "TREATED LIST\n";
        for (auto& t : treatedList)
            std::cout << std::get<0>(t)
                      << "  " 
                      << std::get<1>(t)
                      << std::endl;
    }

    void PatientInfo(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (std::find(waitingList.begin(),
                      waitingList.end(),
                      name) != waitingList.end())
        {
            std::cout << name << " is on waiting list.\n";
            return;
        }

        for (auto& t : treatedList)
        {
            if (std::get<0>(t) == name)
            {
                std::cout << name
                          << " is treated on "
                          << std::get<1>(t)
                          << std::endl;
                return;
            }
        }

        std::cout << name << " is not patient\n";
    }
};

void AddPatients(OrganTransplant& ot)
{
    for (int i = 1; i < 31; i++)
    {
        ot.AddToWaitingList("Patient_" + std::to_string(i));
    }
}

void MovePatients(OrganTransplant& ot)
{
    std::string day;
    for (int i = 10; i < 31; i++)
    {
        if (i < 10)
            day = "0" + std::to_string(i);
        else
            day = std::to_string(i);
        ot.MoveFromWaitingToTreated("Patient_" + std::to_string(i),
                                    day + "-01-2026");
    }
}

void ReadData(OrganTransplant& ot)
{
    for (int i = 1; i < 3; i++)
    {
        ot.PrintWaiting();
        ot.PrintTreated();
    }
}

int main()
{
    OrganTransplant ot;

    std::thread t1(AddPatients, std::ref(ot));
    std::thread t2(MovePatients, std::ref(ot));
    std::thread t3(ReadData, std::ref(ot));

    t1.join();
    t2.join();
    t3.join();

    ot.RemoveTreatedBefore("18-01-2026");

    ot.PatientInfo("Patient_5");
    ot.PatientInfo("Patient_15");
    ot.PatientInfo("Patient_25");

    return 0;
}
