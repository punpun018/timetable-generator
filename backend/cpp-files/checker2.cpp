#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <climits>
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

#define vi vector<int>
#define vb vector<bool>
#define vp vector<pair<int, int>>
#define all(x) x.begin(), x.end()
#define vs vector<string>

struct Subject
{
    int hash, strength;
    string Name;
    set<int> studentsEnrolled;
};

struct Student
{
    string rollNumber;
    int hash, noOfSubjects;
    set<int> subjectsEnrolled;
};

struct summary
{
    string rollNumber;
    vi exams;
    vector<vector<string>> subs;
};

struct Partition
{
    int strength = 0;
    set<int> Subjects;
    set<int> Students;
};

void to_json(json &j, const summary &s)
{
    j = json{
        {"rollNumber", s.rollNumber},
        {"exams", s.exams},
        {"subs", s.subs}};
}

int subjects, edges, students, days, examPerDay, limit, totalSlots;
vector<set<int>> Graph;
vector<Subject> subjectData;
vector<Student> studentData;
vector<Partition> Slots;
vector<vector<set<int>>> mainTT;
vector<vector<Partition>> timeTable;
vector<vi> adjMatrix;
vector<summary> Summary;
json jsonData;

void Input()
{
    subjects = jsonData["adjacencyGraph"]["numberOfCourses"];
    edges = jsonData["adjacencyGraph"]["numberOfEdges"];
    students = jsonData["adjacencyGraph"]["students"].size();
    days = jsonData["adjacencyGraph"]["numberOfDays"];
    examPerDay = jsonData["adjacencyGraph"]["numberOfSlots"];
    limit = jsonData["adjacencyGraph"]["maxStrengthPerSlot"];

    string s;
    int id, str, u, v, wt;
    Graph.resize(subjects);
    subjectData.resize(subjects);
    studentData.resize(students);
    adjMatrix.resize(subjects, vi(subjects));

    for (auto it : jsonData["adjacencyGraph"]["edges"])
    {
        u = it[0], v = it[1], wt = it[2];
        Graph[u].insert(v);
        Graph[v].insert(u);
        adjMatrix[u][v] = adjMatrix[v][u] = wt;
    }

    int i = 0;
    for (auto &[course_name, course_details] : jsonData["adjacencyGraph"]["courses"].items())
    {
        s = course_name;
        id = course_details["id"];
        if (id != i)
            cout << "Fail";
        str = course_details["size"];
        subjectData[i].Name = s;
        subjectData[i].hash = id;
        subjectData[i++].strength = str;
    }

    i = 0;
    for (auto &[x, y] : jsonData["adjacencyGraph"]["students"].items())
    {
        s = x;
        studentData[i].rollNumber = s;
        id = y["id"];
        studentData[i].hash = id;
        str = y["courses"].size();
        studentData[i].noOfSubjects = str;
        for (int it : y["courses"])
            studentData[i].subjectsEnrolled.insert(it);
        i++;
    }
    totalSlots = days * examPerDay;
}

void Input2()
{
    days = jsonData["examTT"].size();
    examPerDay = jsonData["examTT"][0].size();
    mainTT.resize(days, vector<set<int>>(examPerDay));
    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (int it : jsonData["examTT"][i][j])
                mainTT[i][j].insert(it);
        }
    }
}

tuple<int, int> getTwoExams()
{
    int ct1 = 0, ct2 = 0;
    for (int i = 0; i < students; i++)
    {
        ct1 += count(Summary[i].exams.begin(), Summary[i].exams.end(), 2);
        ct2 += count(Summary[i].exams.begin(), Summary[i].exams.end(), 3);
    }
    return {ct1, ct2};
}

tuple<int, int, int, int> getSumm()
{
    int ct1 = 0, ct2 = 0, ct3 = 0, ct4 = 0;
    for (int i = 0, num; i < students; i++)
        for (int j = 0; j < days - 1; j++)
        {
            num = Summary[i].exams[j] + Summary[i].exams[j + 1];
            if (num == 3)
                ct1++;
            if (num == 4)
                ct2++;
            if (num == 5)
                ct3++;
            if (num == 6)
                ct4++;
        }
    return {ct1, ct2, ct3, ct4};
}

void updateTT()
{
    Summary.resize(students);
    timeTable.resize(days, vector<Partition>(examPerDay));

    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (int it : mainTT[i][j])
            {
                timeTable[i][j].Subjects.insert(it);
                timeTable[i][j].strength += subjectData[it].strength;
                for (int st = 0; st < students; st++)
                    if (studentData[st].subjectsEnrolled.find(it) != studentData[st].subjectsEnrolled.end())

                        timeTable[i][j].Students.insert(st);
            }
            if (timeTable[i][j].strength != timeTable[i][j].Students.size())
            {
                cout << "Not Feasible\n";
                exit(0);
            }
        }
    }
    for (int i = 0; i < students; i++)
    {
        Summary[i].exams.resize(days);
        Summary[i].subs.resize(days);
        Summary[i].rollNumber = studentData[i].rollNumber;
        for (int d = 0; d < days; d++)
        {
            for (int j = 0; j < examPerDay; j++)
            {
                if (timeTable[d][j].Students.find(i) != timeTable[d][j].Students.end())

                    Summary[i].exams[d]++;
                for (auto it : timeTable[d][j].Subjects)
                {
                    if (studentData[i].subjectsEnrolled.find(it) != studentData[i].subjectsEnrolled.end())

                        Summary[i].subs[d].push_back(subjectData[it].Name);
                }
            }
        }
    }
}

void outJSON()
{
    vector<vector<pair<int, vs>>> vvs(days, vector<pair<int, vs>>(examPerDay));
    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            int sum = 0;
            for (auto it : mainTT[i][j])
            {
                string temp = " (" + to_string(subjectData[it].strength) + ")";
                vvs[i][j].second.push_back(subjectData[it].Name + temp);
                sum += subjectData[it].strength;
            }
            vvs[i][j].first = sum;
        }
    }

    json outJS;
    outJS["days"] = jsonData["examTT"].size();
    outJS["examTT"] = jsonData["examTT"];
    outJS["subjNames"] = jsonData["subjNames"];
    outJS["timeTable"] = vvs;
    outJS["timeTableSummary"] = Summary;
    auto [x, y] = getTwoExams();
    auto [a, b, c, d] = getSumm();
    outJS["Summary"] = {x, y, a, b, c, d};
    cout << outJS.dump(1) << endl;
}

int main()
{

    cin >> jsonData;
    Input();
    Input2();
    updateTT();
    outJSON();
    return 0;
}