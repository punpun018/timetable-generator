#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include <algorithm>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

#define vi vector<int>
#define vb vector<bool>
#define vp vector<pair<int, int>>
#define vs vector<string>

struct Subject
{
    int hash, strength;
    string Name;
};

struct Student
{
    string rollNumber;
    int hash, noOfSubjects;
    set<int> subjectsEnrolled;
};

struct Partition
{
    int strength = 0;
    set<int> Subjects;
    set<int> Students;
};

struct summary
{
    string rollNumber;
    vi exams;

    friend void to_json(json &j, const summary &s)
    {
        j = json{{"rollNumber", s.rollNumber}, {"exams", s.exams}};
    }

    friend void from_json(const json &j, summary &s)
    {
        j.at("rollNumber").get_to(s.rollNumber);
        j.at("exams").get_to(s.exams);
    }
};

int subjects, edges, students, days, examPerDay;
vector<set<int>> Graph;
vector<Subject> subjectData;
vector<Student> studentData;
vector<vector<Partition>> timeTable;
vector<vector<vs>> ttSchedule;
vector<summary> Summary;

void Input(json &jsonData)
{
    subjects = jsonData["adjacencyGraph"]["num_courses"];
    edges = jsonData["adjacencyGraph"]["num_edges"];
    students = jsonData["adjacencyGraph"]["students"].size();

    Graph.resize(subjects);
    subjectData.resize(subjects);
    studentData.resize(students);

    for (const auto &it : jsonData["adjacencyGraph"]["edges"])
    {
        int u = it[0], v = it[1];
        Graph[u].insert(v);
        Graph[v].insert(u);
    }

    int i = 0;
    for (auto &[course_name, course_details] : jsonData["adjacencyGraph"]["courses"].items())
    {
        subjectData[i].Name = course_name;
        subjectData[i].hash = course_details["id"];
        subjectData[i++].strength = course_details["size"];
    }

    i = 0;
    for (auto &[roll, info] : jsonData["adjacencyGraph"]["students"].items())
    {
        studentData[i].rollNumber = roll;
        studentData[i].hash = info["id"];
        studentData[i].noOfSubjects = info["courses"].size();
        for (int sub : info["courses"])
            studentData[i].subjectsEnrolled.insert(sub);
        i++;
    }

    days = jsonData["adjacencyGraph"]["timeTable"].size();
    examPerDay = jsonData["adjacencyGraph"]["timeTable"][0].size();
    timeTable.resize(days, vector<Partition>(examPerDay));
    ttSchedule.resize(days, vector<vs>(examPerDay));

    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (int it : jsonData["adjacencyGraph"]["timeTable"][i][j])
            {
                ttSchedule[i][j].push_back(subjectData[it].Name);
                timeTable[i][j].Subjects.insert(it);
                timeTable[i][j].strength += subjectData[it].strength;
                for (int st = 0; st < students; st++)
                {
                    if (studentData[st].subjectsEnrolled.count(it))
                        timeTable[i][j].Students.insert(st);
                }
            }
            if (timeTable[i][j].strength != (int)timeTable[i][j].Students.size())
            {
                cout << "Not Feasible\n";
                exit(0);
            }
        }
    }
}

void createSummary()
{
    Summary.resize(students);
    for (int i = 0; i < students; i++)
    {
        Summary[i].exams.resize(days);
        Summary[i].rollNumber = studentData[i].rollNumber;
        for (int d = 0; d < days; d++)
        {
            for (int j = 0; j < examPerDay; j++)
            {
                if (timeTable[d][j].Students.count(i))
                    Summary[i].exams[d]++;
            }
        }
    }
}

int getTwoExams()
{
    int ct = 0;
    for (int i = 0; i < students; i++)
        ct += count(Summary[i].exams.begin(), Summary[i].exams.end(), 2);
    return ct;
}

pair<int, int> getSumm()
{
    int ct1 = 0, ct2 = 0;
    for (int i = 0; i < students; i++)
    {
        for (int j = 0; j < days - 1; j++)
        {
            int num = Summary[i].exams[j] + Summary[i].exams[j + 1];
            if (num == 3)
                ct1++;
            if (num == 4)
                ct2++;
        }
    }
    return {ct1, ct2};
}

int main()
{
    ifstream file("TimeTable_json.txt");
    if (!file.is_open())
    {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    json jsonData;
    file >> jsonData;

    Input(jsonData);
    createSummary();

    json finalJson;
    finalJson["studentExamTable"] = Summary;

    json timeTableJson;
    for (int i = 0; i < ttSchedule.size(); i++)
    {
        json dayJson;
        for (int j = 0; j < ttSchedule[i].size(); j++)
        {
            json slotJson;
            slotJson["Subjects"] = ttSchedule[i][j];
            slotJson["Strength"] = timeTable[i][j].strength;
            dayJson["Slot " + to_string(j + 1)] = slotJson;
        }
        timeTableJson["Day " + to_string(i + 1)] = dayJson;
    }

    finalJson["Schedule"] = timeTableJson;
    finalJson["Two Exams In a Day"] = getTwoExams();
    finalJson["Three Exams In Two Day"] = getSumm().first;
    finalJson["Four Exams In Two Day"] = getSumm().second;

    ofstream outFile("TimeTableSummary.txt");
    outFile << finalJson.dump(2) << endl;
    outFile.close();

    return 0;
}
