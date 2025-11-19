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
    for (int i = 0, num; i < students; i++)
        for (int j = 0; j < days - 1; j++)
        {
            num = Summary[i].exams[j] + Summary[i].exams[j + 1];
            if (num == 3)
                ct1++;
            if (num == 4)
                ct2++;
        }
    return {ct1, ct2};
}

vector<pair<int, int>> getSlots(string &sub)
{
    int subHash = -1;
    pair<int, int> pp;
    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (auto it : mainTT[i][j])
            {
                if (subjectData[it].Name == sub)
                {
                    subHash = it;
                    pp = {i, j};
                    break;
                }
            }
        }
    }

    vector<pair<int, int>> ans;
    if (subHash == -1)
        return ans;
    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            bool flag = true;
            for (auto it : mainTT[i][j])
            {
                if (adjMatrix[it][subHash])
                    flag = false;
            }
            if (i == pp.first && j == pp.second)
                flag = false;
            if (flag)
                ans.push_back({i, j});
        }
    }

    return ans;
}

int main()
{
    cin >> jsonData;
    Input();
    Input2();
    string sub = jsonData["swap"]["subject"];
    json outJson = jsonData;
    outJson["swap"]["validSlots"] = getSlots(sub);
    cout << outJson.dump(2) << endl;
    return 0;
}