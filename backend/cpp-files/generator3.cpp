#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <unordered_set>
#include <tuple>
#include <algorithm>
#include <climits>
#include <numeric>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

#define vi vector<int>
#define vvi vector<vector<int>>
#define vb vector<bool>
#define vp vector<pair<int, int>>
#define all(x) x.begin(), x.end()

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

struct Partition
{
    int strength = 0;
    set<int> Subjects;
    set<int> Students;
};

int subjects, edges, students, days, examPerDay, limit, totalSlots;
vector<set<int>> Graph;
vector<Subject> subjectData;
vector<Student> studentData;
vector<Partition> Slots;
vector<vector<set<int>>> timeTable;
vector<vi> adjMatrix;
vector<vector<Partition>> TT;

json outJson;

void Input(json &jsonData)
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
    // C++14: Replaced structured binding with iterator loop
    for (auto& element : jsonData["adjacencyGraph"]["courses"].items())
    {
        s = element.key();
        auto& course_details = element.value();
        
        id = course_details["id"];
        if (id != i)
            cerr << "Fail"; // Changed to cerr to avoid corrupting JSON output
        str = course_details["size"];
        subjectData[i].Name = s;
        subjectData[i].hash = id;
        subjectData[i++].strength = str;
    }

    i = 0;
    // C++14: Replaced structured binding with iterator loop
    for (auto& element : jsonData["adjacencyGraph"]["students"].items())
    {
        s = element.key();
        auto& y = element.value();

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

void nameJson()
{
    timeTable.resize(days, vector<set<int>>(examPerDay));

    for (int i = 0; i < days; i++)
    {
        for (int j = 0; j < examPerDay; j++)
        {
            for (auto it : TT[i][j].Subjects)
                timeTable[i][j].insert(it);
        }
    }

    vector<vector<vector<string>>> v;
    v.resize(days, vector<vector<string>>(examPerDay));

    for (int i = 0; i < days; i++)
        for (int j = 0; j < examPerDay; j++)
            for (auto it : timeTable[i][j])
                v[i][j].push_back(subjectData[it].Name + " (" + to_string(subjectData[it].strength) + ")");

    outJson["days"] = days;
    outJson["slotsPerDay"] = examPerDay;
    outJson["examTT"] = timeTable;
    outJson["subjNames"] = v;
}

void optimizeTT()
{
    for (int i = 0; i < days; i++)
    {
        for (int s = 0; s < 4; s++)
        {
            int ct = 50;
            while (ct-- && TT[i][s].strength > limit)
            {
                for (auto it : TT[i][s].Subjects)
                {
                    for (int j = 0; j < 4; j++)
                        if (j != s)
                        {
                            bool canPlace = true;
                            for (auto itr : TT[i][j].Subjects)
                                if (adjMatrix[it][itr])
                                    canPlace = false;
                            if (canPlace)
                            {
                                TT[i][s].Subjects.erase(it);
                                TT[i][j].Subjects.insert(it);
                                TT[i][s].strength -= subjectData[it].strength;
                                TT[i][j].strength += subjectData[it].strength;
                                goto nextIter;
                            }
                        }
                }
            nextIter:;
            }
        }
    }

    set<int> clashes;
    for (int i = 0; i < days; i++)
        for (int a = 0; a < 4; a++)
            for (int b = a + 1; b < 4; b++)
                for (auto it : TT[i][a].Subjects)
                    for (auto itr : TT[i][b].Subjects)
                        if (adjMatrix[it][itr])
                        {
                            clashes.insert(it);
                            clashes.insert(itr);
                        }

    vector<int> vec;
    for (auto it : clashes)
        vec.push_back(it);
    for (auto it : clashes)
        vec.push_back(it);

    for (auto it : vec)
    {
        for (int i = 0; i < days; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                // C++14: Replaced .contains() with .count() > 0
                if (TT[i][j].Subjects.count(it) > 0)
                {
                    TT[i][j].Subjects.erase(it);
                    TT[i][j].strength -= subjectData[it].strength;
                    goto removed;
                }
            }
        }
    removed:;

        int minClashes = INT_MAX, cx = 0, cy = 0;
        for (int i = 0; i < days; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                bool canPlace = true;
                for (auto itr : TT[i][j].Subjects)
                    if (adjMatrix[it][itr])
                        canPlace = false;
                if (canPlace)
                {
                    int tempClashes = 0;
                    for (int k = 0; k < 4; k++)
                        if (k != j)
                            for (auto itr : TT[i][k].Subjects)
                                tempClashes += adjMatrix[it][itr];
                    if (tempClashes < minClashes)
                        minClashes = tempClashes, cx = i, cy = j;
                }
            }
        }
        TT[cx][cy].Subjects.insert(it);
        TT[cx][cy].strength += subjectData[it].strength;
    }

    for (int i = 0; i < days; i++)
        sort(TT[i].begin(), TT[i].end(),
             [](const Partition &a, const Partition &b)
             {
                 return a.strength > b.strength;
             });

    for (int i = 0; i < days; i++)
    {
        for (int s = 0; s < 4; s++)
        {
            int ct = 50;
            while (ct-- && TT[i][s].strength > limit)
            {
                for (auto it : TT[i][s].Subjects)
                {
                    for (int j = 0; j < 4; j++)
                        if (j != s)
                        {
                            bool canPlace = true;
                            for (auto itr : TT[i][j].Subjects)
                                if (adjMatrix[it][itr])
                                    canPlace = false;
                            if (canPlace)
                            {
                                TT[i][s].Subjects.erase(it);
                                TT[i][j].Subjects.insert(it);
                                TT[i][s].strength -= subjectData[it].strength;
                                TT[i][j].strength += subjectData[it].strength;
                                goto nextIter2;
                            }
                        }
                }
            nextIter2:;
            }
        }
    }

    for (int i = 0; i < days; i++)
    {
        for (int s = 0; s < 4; s++)
        {
            while (TT[i][s].strength > limit)
            {
                bool moved = false;
                for (auto it : TT[i][s].Subjects)
                {
                    for (int j = 0; j < 4; j++)
                        if (j != s && TT[i][j].strength + subjectData[it].strength <= limit)
                        {
                            bool canPlace = true;
                            for (auto itr : TT[i][j].Subjects)
                                if (adjMatrix[it][itr])
                                {
                                    canPlace = false;
                                    break;
                                }
                            if (canPlace)
                            {
                                TT[i][s].Subjects.erase(it);
                                TT[i][j].Subjects.insert(it);
                                TT[i][s].strength -= subjectData[it].strength;
                                TT[i][j].strength += subjectData[it].strength;
                                moved = true;
                                goto nextMove;
                            }
                        }
                }
            nextMove:;
                if (!moved)
                    break;
            }
        }
    }
}

vector<tuple<int, int, int, int>> minimizeSlotClashes4(const vvi &confMatrix,
                                                       const vector<vvi> &confMatrix2,
                                                       const vector<vector<vector<vector<int>>>> &confMatrix3)
{
    int n = confMatrix.size();
    if (n % 4 != 0)
    {
        throw invalid_argument("Number of slots must be divisible by 4.");
    }

    vector<bool> used(n, false);
    vector<tuple<int, int, int, int>> result;

    for (int iter = 0; iter < n / 4; ++iter)
    {
        int i = -1;
        int maxSum = -1;
        for (int x = 0; x < n; ++x)
            if (!used[x])
            {
                int sum = 0;
                for (int y = 0; y < n; ++y)
                    if (!used[y])
                        sum += confMatrix[x][y];
                if (sum > maxSum)
                    maxSum = sum, i = x;
            }

        int bestj = -1, bestk = -1, bestl = -1;
        int bestCost = INT_MAX;

        for (int j = 0; j < n; ++j)
            if (!used[j] && j != i)
                for (int k = j + 1; k < n; ++k)
                    if (!used[k] && k != i)
                        for (int l = k + 1; l < n; ++l)
                            if (!used[l] && l != i)
                            {

                                int cost = 10000 * confMatrix3[i][j][k][l] + 500 * (confMatrix2[i][j][k] + confMatrix2[i][j][l] + confMatrix2[i][k][l] + confMatrix2[j][k][l]) + confMatrix[i][j] + confMatrix[i][k] + confMatrix[i][l] + confMatrix[j][k] + confMatrix[j][l] + confMatrix[k][l];

                                if (cost < bestCost)
                                {
                                    bestCost = cost;
                                    bestj = j;
                                    bestk = k;
                                    bestl = l;
                                }
                            }

        if (i == -1 || bestj == -1 || bestk == -1 || bestl == -1)
            break;

        result.emplace_back(i, bestj, bestk, bestl);
        used[i] = used[bestj] = used[bestk] = used[bestl] = true;
    }

    return result;
}

vector<vector<vector<vector<int>>>> buildConfMatrix4(const vector<Partition> &Slots, const vector<Student> &studentData)
{
    int n = Slots.size();
    vector<vector<vector<vector<int>>>> confMatrix3(n, vector<vector<vector<int>>>(n, vector<vector<int>>(n, vector<int>(n, 0))));

    for (const auto &student : studentData)
    {
        vector<int> enrolledSlots;
        for (int subj : student.subjectsEnrolled)
        {
            for (int s = 0; s < n; s++)
            {
                if (Slots[s].Subjects.count(subj))
                {
                    enrolledSlots.push_back(s);
                }
            }
        }

        int m = enrolledSlots.size();
        for (int a = 0; a < m; a++)
        {
            for (int b = a + 1; b < m; b++)
            {
                for (int c = b + 1; c < m; c++)
                {
                    for (int d = c + 1; d < m; d++)
                    {
                        int i = enrolledSlots[a];
                        int j = enrolledSlots[b];
                        int k = enrolledSlots[c];
                        int l = enrolledSlots[d];
                        confMatrix3[i][j][k][l]++;
                        confMatrix3[i][j][l][k]++;
                        confMatrix3[i][k][j][l]++;
                        confMatrix3[i][k][l][j]++;
                        confMatrix3[i][l][j][k]++;
                        confMatrix3[i][l][k][j]++;

                        confMatrix3[j][i][k][l]++;
                        confMatrix3[j][i][l][k]++;
                        confMatrix3[j][k][i][l]++;
                        confMatrix3[j][k][l][i]++;
                        confMatrix3[j][l][i][k]++;
                        confMatrix3[j][l][k][i]++;

                        confMatrix3[k][i][j][l]++;
                        confMatrix3[k][i][l][j]++;
                        confMatrix3[k][j][i][l]++;
                        confMatrix3[k][j][l][i]++;
                        confMatrix3[k][l][i][j]++;
                        confMatrix3[k][l][j][i]++;

                        confMatrix3[l][i][j][k]++;
                        confMatrix3[l][i][k][j]++;
                        confMatrix3[l][j][i][k]++;
                        confMatrix3[l][j][k][i]++;
                        confMatrix3[l][k][i][j]++;
                        confMatrix3[l][k][j][i]++;
                    }
                }
            }
        }
    }

    return confMatrix3;
}

vector<vvi> buildConfMatrix3(const vector<Partition> &Slots, const vector<Student> &studentData)
{
    int n = Slots.size();
    vector<vector<vector<int>>> confMatrix2(n, vector<vector<int>>(n, vector<int>(n, 0)));

    for (const auto &student : studentData)
    {
        vector<int> enrolledSlots;
        for (int subj : student.subjectsEnrolled)
        {
            for (int s = 0; s < n; s++)
            {
                if (Slots[s].Subjects.count(subj))
                {
                    enrolledSlots.push_back(s);
                }
            }
        }

        int m = enrolledSlots.size();
        for (int a = 0; a < m; a++)
        {
            for (int b = a + 1; b < m; b++)
            {
                for (int c = b + 1; c < m; c++)
                {
                    int i = enrolledSlots[a];
                    int j = enrolledSlots[b];
                    int k = enrolledSlots[c];
                    confMatrix2[i][j][k]++;
                    confMatrix2[i][k][j]++;
                    confMatrix2[j][i][k]++;
                    confMatrix2[j][k][i]++;
                    confMatrix2[k][i][j]++;
                    confMatrix2[k][j][i]++;
                }
            }
        }
    }

    return confMatrix2;
}

void makeTT4()
{
    int n = totalSlots;
    vector<vi> confMatrix(n, vi(n, 0));
    for (int i = 0; i < n; i++)
    {
        for (int j = i; j < n; j++)
        {
            int clashes = 0;
            for (auto it : Slots[i].Subjects)
                for (auto itr : Slots[j].Subjects)
                    clashes += adjMatrix[it][itr];
            confMatrix[i][j] = confMatrix[j][i] = clashes;
        }
    }

    auto confMatrix2 = buildConfMatrix3(Slots, studentData);
    auto confMatrix3 = buildConfMatrix4(Slots, studentData);
    auto quadruplets = minimizeSlotClashes4(confMatrix, confMatrix2, confMatrix3);

    TT.clear();
    TT.resize(days, vector<Partition>(examPerDay));

    for (int d = 0; d < days; d++)
    {
        // C++14: Replaced structured binding with std::tie
        int a, b, c, e;
        tie(a, b, c, e) = quadruplets[d];

        for (auto it : Slots[a].Subjects)
        {
            TT[d][0].Subjects.insert(it);
            TT[d][0].strength += subjectData[it].strength;
        }
        for (auto it : Slots[b].Subjects)
        {
            TT[d][1].Subjects.insert(it);
            TT[d][1].strength += subjectData[it].strength;
        }
        for (auto it : Slots[c].Subjects)
        {
            TT[d][2].Subjects.insert(it);
            TT[d][2].strength += subjectData[it].strength;
        }
        for (auto it : Slots[e].Subjects)
        {
            TT[d][3].Subjects.insert(it);
            TT[d][3].strength += subjectData[it].strength;
        }
        sort(TT[d].begin(), TT[d].end(),
             [](const Partition &p1, const Partition &p2)
             {
                 return p1.strength > p2.strength;
             });
    }
}

void genrateSlots()
{
    int NUM_SLOTS = days * examPerDay;
    int n = adjMatrix.size();
    vector<vi> slots(NUM_SLOTS);
    vi slotWt(NUM_SLOTS, 0);
    vi subjSlot(n, -1);

    vi degree(n, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            degree[i] += adjMatrix[i][j];

    vi subjects(n);
    iota(subjects.begin(), subjects.end(), 0);
    sort(subjects.begin(), subjects.end(), [&](int a, int b)
         { return degree[a] > degree[b]; });

    for (int u : subjects)
    {
        int bestSlot = -1;
        int minInc = INT_MAX;

        for (int s = 0; s < NUM_SLOTS; ++s)
        {
            int intraClash = 0;

            for (int v : slots[s])
            {
                intraClash += adjMatrix[u][v];
            }

            if (intraClash < minInc)
            {
                minInc = intraClash;
                bestSlot = s;
            }
        }

        slots[bestSlot].push_back(u);
        subjSlot[u] = bestSlot;
    }

    Slots.resize(slots.size());
    for (int i = 0; i < slots.size(); i++)
        for (auto it : slots[i])
        {
            Slots[i].Subjects.insert(it);
            Slots[i].strength += subjectData[it].strength;
        }
}

void Print()
{
    int count = 0;
    for (auto it : TT)
    {
        count++;
        cout << "Day -: " << count << "\n";
        cout << "Slot 1 : " << it[0].strength << " ";
        for (auto itr : it[0].Subjects)
            cout << subjectData[itr].Name << " ";
        cout << "\nSlot 2 : " << it[1].strength << " ";
        for (auto itr : it[1].Subjects)
            cout << subjectData[itr].Name << " ";
        cout << "\n";
    }
}

vi minimizeDayClashes(const vector<vi> &confMatrix)
{
    int n = confMatrix.size();
    vector<bool> used(n, false);
    vi order;

    int curr = 0;
    order.push_back(curr);
    used[curr] = true;

    for (int step = 1; step < n; ++step)
    {
        int nextDay = -1, minClash = INT_MAX;
        for (int j = 0; j < n; ++j)
        {
            if (!used[j] && confMatrix[curr][j] < minClash)
            {
                minClash = confMatrix[curr][j];
                nextDay = j;
            }
        }
        if (nextDay != -1)
        {
            order.push_back(nextDay);
            used[nextDay] = true;
            curr = nextDay;
        }
    }

    auto calcClash = [&](const vi &ord)
    {
        int sum = 0;
        for (int i = 0; i + 1 < n; ++i)
            sum += confMatrix[ord[i]][ord[i + 1]];
        return sum;
    };

    int bestClash = calcClash(order);
    bool improved = true;

    while (improved)
    {
        improved = false;
        for (int i = 0; i < n; ++i)
        {
            for (int j = i + 1; j < n; ++j)
            {
                swap(order[i], order[j]);
                int newClash = calcClash(order);
                if (newClash < bestClash)
                {
                    bestClash = newClash;
                    improved = true;
                }
                else
                {
                    swap(order[i], order[j]);
                }
            }
        }
    }

    return order;
}

void finalOptimization()
{
    vector<vi> confMatrix(days, vi(days));
    for (int i = 0; i < students; i++)
    {
        for (int d1 = 0; d1 < days; d1++)
        {
            int exams = 0;
            for (int sl = 0; sl < 4; sl++)
            {
                for (auto it : TT[d1][sl].Subjects)
                {
                    // C++14: Replaced .contains() with .count()
                    if (studentData[i].subjectsEnrolled.count(it))
                    {
                        exams++;
                        break;
                    }
                }
            }

            for (int d2 = d1 + 1; d2 < days; d2++)
            {
                int exam2 = 0;
                for (int sl = 0; sl < 4; sl++)
                {
                    for (auto it : TT[d2][sl].Subjects)
                    {
                        // C++14: Replaced .contains() with .count()
                        if (studentData[i].subjectsEnrolled.count(it))
                        {
                            exam2++;
                            break;
                        }
                    }
                }

                int total = exams + exam2;
                if (total == 4)
                {
                    confMatrix[d1][d2] += 1;
                    confMatrix[d2][d1] += 1;
                }
                if (total == 5)
                {
                    confMatrix[d1][d2] += 50;
                    confMatrix[d2][d1] += 50;
                }
                if (total == 6)
                {
                    confMatrix[d1][d2] += 1000;
                    confMatrix[d2][d1] += 1000;
                }
                if (total == 7)
                {
                    confMatrix[d1][d2] += 10000;
                    confMatrix[d2][d1] += 10000;
                }
                if (total == 8)
                {
                    confMatrix[d1][d2] += 100000;
                    confMatrix[d2][d1] += 100000;
                }
            }
        }
    }

    vi order = minimizeDayClashes(confMatrix);
    vector<vector<Partition>> temp = TT;
    for (int i = 0; i < days; i++)
        temp[i] = TT[order[i]];
    TT = temp;
}

int main()
{
    json jsonData;
    cin >> jsonData;
    Input(jsonData);
    outJson = jsonData;
    genrateSlots();
    makeTT4();
    optimizeTT();
    optimizeTT();
    finalOptimization();
    nameJson();
    cout << outJson.dump(2);
    return 0;
}