import { useLocation } from "react-router-dom";
import React, { useState } from "react";
import "../styles/TimetablePage.css";

const TimetablePage = () => {
    const location = useLocation();
    // const timetableData = location.state?.timetableData;
    const [validSlots, setValidSlots] = useState(null);
    const [selectedSubject, setSelectedSubject] = useState(null);
    const [timetableData, setTimetableData] = useState(location.state?.timetableData || {});

    if (!timetableData || Object.keys(timetableData).length === 0) {
        return <p className="p-4 text-red-500">No timetable data received.</p>;
    }

    const handleDownload = async () => {
        try {
            const response = await fetch("http://localhost:5000/excel/generate-excel", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(timetableData),
            });

            if (!response.ok) throw new Error("Failed to download Excel");

            const blob = await response.blob();
            const url = window.URL.createObjectURL(blob);
            const link = document.createElement("a");
            link.href = url;
            link.setAttribute("download", "TimeTable.xlsx");
            document.body.appendChild(link);
            link.click();
            link.remove();
        } catch (err) {
            console.error("Download failed:", err);
            alert("Failed to download Excel");
        }
    };

    const handleSubjectClick = async (subjectName) => {
        try {
            const cleanName = subjectName.split(" (")[0]; // Remove the number in parentheses
            const response = await fetch("http://localhost:5000/api/slots", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify({
                    ...timetableData,
                    swap: { subject: cleanName },
                }),
            });
            const data = await response.json();
            console.log(data.swap.subject);
            console.log(data.swap.validSlots);
            setValidSlots(data.swap.validSlots);
            setSelectedSubject(cleanName);
        } catch (err) {
            console.error("Error fetching valid slots:", err);
        }
    };

    const handleMoveSubject = async (newDay, newSlot) => {
        try {
            const updatedTimetable = JSON.parse(JSON.stringify(timetableData));

            const baseName = selectedSubject.split(" (")[0];
            const subjectEntry = updatedTimetable.adjacencyGraph.courses[baseName];
            if (!subjectEntry) throw new Error("Subject not found in adjacency graph");

            const subjectId = subjectEntry.id;

            // --- Remove from old position in examTT and subjNames ---
            let found = false;
            for (let d = 0; d < updatedTimetable.examTT.length; d++) {
                for (let s = 0; s < updatedTimetable.examTT[d].length; s++) {
                    const idIndex = updatedTimetable.examTT[d][s].indexOf(subjectId);
                    const nameIndex = updatedTimetable.subjNames[d][s].indexOf(selectedSubject);

                    if (idIndex !== -1) {
                        updatedTimetable.examTT[d][s].splice(idIndex, 1);
                        if (nameIndex !== -1) {
                            updatedTimetable.subjNames[d][s].splice(nameIndex, 1);
                        }
                        found = true;
                        break;
                    }
                }
                if (found) break;
            }

            // --- Add to new position ---
            updatedTimetable.examTT[newDay][newSlot].push(subjectId);
            updatedTimetable.subjNames[newDay][newSlot].push(selectedSubject);

            // --- Send to checker ---
            const response = await fetch("http://localhost:5000/api/checker", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(updatedTimetable),
            });

            if (!response.ok) throw new Error("Checker failed");

            const newCheckedData = await response.json();

            setValidSlots([]);
            setSelectedSubject(null);
            setTimetableData(newCheckedData);
        } catch (err) {
            console.error("Failed to move subject:", err);
            alert("Move failed.");
        }
    };


    return (
        <div className="timetable-container">
            <h2 className="text-2xl font-bold mb-4">Timetable</h2>
            <table className="timetable-table">
                <thead>
                    <tr>
                        <th>Day</th>
                        {[...Array(timetableData.slotsPerDay)].map((_, slotIdx) => (
                            <th key={slotIdx}>Slot {slotIdx + 1}</th>
                        ))}
                    </tr>
                </thead>
                <tbody>
                    {timetableData.subjNames.map((daySlots, dayIdx) => (
                        <tr key={dayIdx}>
                            <td><strong>Day {dayIdx + 1}</strong></td>
                            {daySlots.map((subjects, slotIdx) => (
                                <td key={slotIdx}>
                                    <div className="subject-grid">
                                        {subjects.length > 0 ? (
                                            subjects.map((subj, idx) => (
                                                <button
                                                    key={idx}
                                                    className="subject-pill"
                                                    onClick={() => handleSubjectClick(subj)}
                                                >
                                                    {subj}
                                                </button>
                                            ))
                                        ) : (
                                            <span className="text-gray-400 text-sm">No exams</span>
                                        )}
                                    </div>
                                </td>
                            ))}
                        </tr>
                    ))}
                </tbody>
            </table>

            {/* Checker Summary */}
            <div style={{ display: "flex", alignItems: "center", width: "100%", justifyContent: "center" }}>
                {Array.isArray(timetableData.Summary) && timetableData.Summary.length === 3 && (
                    <div className="summary-container">
                        <div className="summary-box">
                            <p><strong>Students with exactly 2 exams on a day:</strong> {timetableData.Summary[0]}</p>
                            <p><strong>Students with 3 exams on two consecutive days:</strong> {timetableData.Summary[1]}</p>
                            <p><strong>Students with 4 exams on two consecutive days:</strong> {timetableData.Summary[2]}</p>
                        </div>

                        {/* ✅ New Valid Slots Box */}
                        {validSlots && (
                            <div className="summary-box" style={{ marginTop: "20px" }}>
                                <p><strong>Valid slots for "{selectedSubject}":</strong></p>
                                <ul>
                                    {validSlots.map(([day, slot], idx) => (
                                        <li key={idx} style={{ marginBottom: "8px" }}>
                                            Day {day + 1}, Slot {slot + 1}{" "}
                                            <button
                                                onClick={() => handleMoveSubject(day, slot)}
                                                style={{
                                                    marginLeft: "10px",
                                                    padding: "4px 8px",
                                                    backgroundColor: "#4CAF50",
                                                    color: "white",
                                                    border: "none",
                                                    borderRadius: "4px",
                                                    cursor: "pointer"
                                                }}
                                            >
                                                Move
                                            </button>
                                        </li>
                                    ))}
                                </ul>

                            </div>
                        )}
                    </div>
                )}
            </div>

            <div style={{ display: "flex", alignItems: "center", width: "100%", justifyContent: "center" }}>
                <button className="download-btn" onClick={handleDownload}>
                    Download Excel
                </button>
            </div>
        </div>
    );
};

export default TimetablePage;
