import { useLocation } from "react-router-dom";
import React, { useState } from "react";
import "../styles/TimetablePage.css";

const TimetablePage = () => {
    const location = useLocation();
    // const timetableData = location.state?.timetableData;
    const [validSlots, setValidSlots] = useState(null);
    const [selectedSubject, setSelectedSubject] = useState(null);
    const [timetableData, setTimetableData] = useState(location.state?.timetableData || {});
    const [swapDay1, setSwapDay1] = useState("");
    const [swapDay2, setSwapDay2] = useState("");
    const [swapSlotDayA, setSwapSlotDayA] = useState("");
    const [swapSlotDayB, setSwapSlotDayB] = useState("");
    const [swapSlotA, setSwapSlotA] = useState("");
    const [swapSlotB, setSwapSlotB] = useState("");
    const [history, setHistory] = useState([]);
    const [redoStack, setRedoStack] = useState([]);





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

            // --- Try to find a fullName by matching baseName with the start of a name ---
            let fullName = null;

            for (let d = 0; d < updatedTimetable.subjNames.length; d++) {
                for (let s = 0; s < updatedTimetable.subjNames[d].length; s++) {
                    for (const name of updatedTimetable.subjNames[d][s]) {
                        const baseFromName = name.split(" (")[0]; // Get base name from full subject
                        if (baseFromName === baseName) {
                            fullName = name;
                            break;
                        }
                    }
                    if (fullName) break;
                }
                if (fullName) break;
            }

            if (!fullName) throw new Error("Full subject name not found in subjNames");


            // --- Remove from old position in examTT and subjNames ---
            let found = false;
            for (let d = 0; d < updatedTimetable.examTT.length; d++) {
                for (let s = 0; s < updatedTimetable.examTT[d].length; s++) {
                    const idIndex = updatedTimetable.examTT[d][s].indexOf(subjectId);
                    const nameIndex = updatedTimetable.subjNames[d][s].indexOf(fullName);

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
            updatedTimetable.subjNames[newDay][newSlot].push(fullName);

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
            // inside handleMoveSubject or similar
            saveToHistory(newCheckedData);


            setValidSlots([]);
            setSelectedSubject(null);
            setTimetableData(newCheckedData);
        } catch (err) {
            console.error("Failed to move subject:", err);
            alert("Move failed.");
        }
    };

    // const handleSwapDays = () => {
    //     const d1 = parseInt(swapDay1) - 1; // convert to 0-indexed
    //     const d2 = parseInt(swapDay2) - 1;

    //     if (
    //         isNaN(d1) || isNaN(d2) ||
    //         d1 < 0 || d1 >= timetableData.examTT.length ||
    //         d2 < 0 || d2 >= timetableData.examTT.length
    //     ) {
    //         alert("Invalid day numbers");
    //         return;
    //     }

    //     // Save the original state before any modification into history
    //     const originalState = JSON.parse(JSON.stringify(timetableData));
    //     saveToHistory(originalState);  // Save pre-swap state

    //     const updated = { ...timetableData };

    //     // Swap subjNames
    //     const tempNames = updated.subjNames[d1];
    //     updated.subjNames[d1] = updated.subjNames[d2];
    //     updated.subjNames[d2] = tempNames;

    //     // Swap examTT
    //     const tempTT = updated.examTT[d1];
    //     updated.examTT[d1] = updated.examTT[d2];
    //     updated.examTT[d2] = tempTT;

    //     // Optional: if the backend checker is needed after swap
    //     fetch("http://localhost:5000/api/checker", {
    //         method: "POST",
    //         headers: { "Content-Type": "application/json" },
    //         body: JSON.stringify(updated),
    //     })
    //         .then(res => res.json())
    //         .then((checked) => {
    //             // Only update the state after receiving the validated timetable
    //             setTimetableData(checked);
    //             setSwapDay1("");
    //             setSwapDay2("");
    //         })
    //         .catch(err => {
    //             console.error("Swap check failed", err);
    //             alert("Failed to validate swapped days");
    //         });
    // };



    const handleSwapAnySlots = async () => {
        try {
            const dayA = Number(swapSlotDayA) - 1;
            const dayB = Number(swapSlotDayB) - 1;
            const slotA = Number(swapSlotA) - 1;
            const slotB = Number(swapSlotB) - 1;

            const updatedTimetable = JSON.parse(JSON.stringify(timetableData));

            if (
                dayA < 0 || dayA >= updatedTimetable.examTT.length ||
                dayB < 0 || dayB >= updatedTimetable.examTT.length ||
                slotA < 0 || slotA >= updatedTimetable.slotsPerDay ||
                slotB < 0 || slotB >= updatedTimetable.slotsPerDay
            ) {
                alert("Invalid day or slot numbers.");
                return;
            }

            // Save the original state before any modification into history
            const originalState = JSON.parse(JSON.stringify(timetableData));
            saveToHistory(originalState);  // Save pre-swap state

            // Swap examTT
            const tempExam = updatedTimetable.examTT[dayA][slotA];
            updatedTimetable.examTT[dayA][slotA] = updatedTimetable.examTT[dayB][slotB];
            updatedTimetable.examTT[dayB][slotB] = tempExam;

            // Swap subjNames
            const tempNames = updatedTimetable.subjNames[dayA][slotA];
            updatedTimetable.subjNames[dayA][slotA] = updatedTimetable.subjNames[dayB][slotB];
            updatedTimetable.subjNames[dayB][slotB] = tempNames;

            // Send to checker
            const response = await fetch("http://localhost:5000/api/checker", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(updatedTimetable),
            });

            if (!response.ok) throw new Error("Checker failed");

            const newCheckedData = await response.json();

            // Save to history here after the fetch response
            saveToHistory(newCheckedData);

            setTimetableData(newCheckedData);
            setSwapSlotDayA("");
            setSwapSlotDayB("");
            setSwapSlotA("");
            setSwapSlotB("");
        } catch (err) {
            console.error("Failed to swap slots between days:", err);
            alert("Swap between days failed.");
        }
    };


    const saveToHistory = (newData) => {
        setHistory(prev => [...prev, timetableData]);
        setRedoStack([]); // Clear redo stack on new action
        setTimetableData(newData);
    };

    const handleUndo = () => {
        if (history.length === 0) return;
        const prevState = history[history.length - 1];
        setHistory(history.slice(0, -1));
        setRedoStack([timetableData, ...redoStack]);
        setTimetableData(prevState);
    };

    const handleRedo = () => {
        if (redoStack.length === 0) return;
        const nextState = redoStack[0];
        setRedoStack(redoStack.slice(1));
        setHistory([...history, timetableData]);
        setTimetableData(nextState);
    };


    return (
        <div className="timetable-container">
            <h1 className="text-3xl font-bold mb-4 x">TIMETABLE</h1>
            <table className="timetable-table">
                <thead>
                    <tr>
                        <th>DAY</th>
                        {[...Array(timetableData.slotsPerDay)].map((_, slotIdx) => (
                            <th key={slotIdx}>SLOT {slotIdx + 1}</th>
                        ))}
                    </tr>
                </thead>
                <tbody>
                    {timetableData.subjNames.map((daySlots, dayIdx) => (
                        <tr key={dayIdx}>
                            <td><strong>DAY {dayIdx + 1}</strong></td>
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
                        <div className="summary-container2">
                            <div className="summary-box low">
                                <p>Students with exactly 2 exams on a day:</p>
                                <p><strong>{timetableData.Summary[0]} instances</strong></p>
                            </div>
                            <div className="summary-box medium">
                                <p>Students with 3 exams on two consecutive days:</p>
                                <p><strong>{timetableData.Summary[1]} instances</strong></p>
                            </div>
                            <div className="summary-box high">
                                <p>Students with 4 exams on two consecutive days:</p>
                                <p><strong>{timetableData.Summary[2]} instances</strong></p>
                            </div>
                        </div>

                        {validSlots && (
                            <div className="summary-box" style={{ marginTop: "20px" }}>
                                <p><strong>Valid slots for "{selectedSubject}":</strong></p>
                                <ul style={{ padding: 0, listStyle: "none" }}>
                                    {validSlots.map(([day, slot], idx) => (
                                        <li
                                            key={idx}
                                            style={{
                                                marginBottom: "8px",
                                                display: "flex",
                                                justifyContent: "space-between",
                                                alignItems: "center",
                                                gap: "10px"
                                            }}
                                        >
                                            <span>Day {day + 1}, Slot {slot + 1}</span>
                                            <button
                                                onClick={() => handleMoveSubject(day, slot)}
                                                style={{
                                                    padding: "4px 10px",
                                                    backgroundColor: "#4CAF50",
                                                    color: "white",
                                                    border: "none",
                                                    borderRadius: "4px",
                                                    cursor: "pointer",
                                                    whiteSpace: "nowrap"
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

            {/* <div className="swap-days-container">
                <h3>Swap Days</h3>
                <input
                    type="number"
                    placeholder="Day 1"
                    value={swapDay1}
                    onChange={(e) => setSwapDay1(e.target.value)}
                    className="swap-day-input"
                />
                <input
                    type="number"
                    placeholder="Day 2"
                    value={swapDay2}
                    onChange={(e) => setSwapDay2(e.target.value)}
                    className="swap-day-input"
                />
                <button className="swap-day-button" onClick={handleSwapDays}>
                    Swap Days
                </button>
            </div> */}

            <div className="summary-box" style={{ marginTop: "20px" }}>
                <h3 className="font-semibold mb-2">Swap Slots Between Days</h3>
                <div className="flex flex-col gap-2">
                    <input
                        type="number"
                        placeholder="Day A number (1-based)"
                        value={swapSlotDayA}
                        onChange={(e) => setSwapSlotDayA(Number(e.target.value))}
                        className="input-field"
                    />
                    <input
                        type="number"
                        placeholder="Slot A number (1-based)"
                        value={swapSlotA}
                        onChange={(e) => setSwapSlotA(Number(e.target.value))}
                        className="input-field"
                    />
                    <input
                        type="number"
                        placeholder="Day B number (1-based)"
                        value={swapSlotDayB}
                        onChange={(e) => setSwapSlotDayB(Number(e.target.value))}
                        className="input-field"
                    />
                    <input
                        type="number"
                        placeholder="Slot B number (1-based)"
                        value={swapSlotB}
                        onChange={(e) => setSwapSlotB(Number(e.target.value))}
                        className="input-field"
                    />
                    <div className="swap-button-container">
                        <button onClick={handleSwapAnySlots} className="swap-button">
                            Swap
                        </button>
                    </div>
                </div>
            </div>


            <button className="undo-button" onClick={handleUndo} disabled={history.length === 0}>
                ↺ Undo
            </button>
            <button
                className="redo-button"
                onClick={handleRedo}
                disabled={redoStack.length === 0}
                style={{ marginLeft: "20px" }}
            >
                ↻ Redo
            </button>

            <div style={{ display: "flex", alignItems: "center", width: "100%", justifyContent: "center" }}>
                <button className="download-btn" onClick={handleDownload}>
                    Download Excel
                </button>
            </div>
        </div>
    );
};

export default TimetablePage;
