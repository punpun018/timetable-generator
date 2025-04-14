import sys
import json
import pandas as pd
from sortedcontainers import SortedDict, SortedSet

def parse_excel(excel_file: str) -> dict:

    df = pd.read_excel(excel_file, engine="openpyxl")

    courses = SortedDict()

    students = SortedDict()

    for column in df.columns:
        courses[column] = -1
        for i, student in enumerate(df[column].dropna()):
            students[student] = -1

    for i, column in enumerate(courses):
        courses[column] = i

    for i, student in enumerate(students):
        students[student] = i

    courseSize = [0] * len(courses)

    for column in df.columns:
        courseSize[courses[column]] = len(df[column].dropna())

    courseStudents = [SortedSet() for _ in range(len(courses))]

    for col in df.columns:
        col_index = courses[col]
        for student in df[col].dropna():
            courseStudents[col_index].add(students[student])

    studentCourses = [SortedSet() for _ in range(len(students))]

    for col in df.columns:
        col_index = courses[col]
        for student in df[col].dropna():
            studentCourses[students[student]].add(col_index)

    # collection of edges which can not be together
    graphEdges = SortedSet()

    for i, students_i in enumerate(courseStudents):
            for j in range(i + 1, len(courseStudents)):
                num_common_students = len(students_i & courseStudents[j])
                if num_common_students > 0:
                    graphEdges.add((i, j, num_common_students))

    return {
        "numberOfDays": -1,
        "numberOfSlots": -1,
        "numberOfCourses": len(courses),
        "numberOfEdges": len(graphEdges),
        "edges": list(graphEdges),
        "courses": {column: {"id": i, "size": courseSize[i]} for column, i in courses.items()},
        "students": {student: {"id": i, "courses": list(studentCourses[i])} for student, i in students.items()}
    }


if __name__ == "__main__":
    input_file = sys.stdin.buffer.read()
    temp_file = "temp.xlsx"

    with open(temp_file, "wb") as f:
        f.write(input_file)

    result = parse_excel(temp_file)

    # ✅ Wrap inside "adjacencyGraph"
    print(json.dumps({ "adjacencyGraph": result }))