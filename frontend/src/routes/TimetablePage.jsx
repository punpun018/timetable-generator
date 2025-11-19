import { useLocation } from "react-router-dom";
import React, { useState, useEffect } from "react";
import axios from "axios";
import "../styles/TimetablePage.css";

const production = import.meta.env.VITE_PRODUCTION;
const BASE_URL =
    production === "true"
        ? import.meta.env.VITE_BASE_URL_BACKEND
        : "http://localhost:5000";

const TimetablePage = () => {
    const location = useLocation();
    const [validSlots, setValidSlots] = useState(null);
    const [selectedSubject, setSelectedSubject] = useState(null);
    const [timetableData, setTimetableData] = useState(
        location.state?.timetableData || {}
    );

    // Safe initialization of stats
    const [stats, setStats] = useState(null);

    const [swapSlotDayA, setSwapSlotDayA] = useState("");
    const [swapSlotDayB, setSwapSlotDayB] = useState("");
    const [swapSlotA, setSwapSlotA] = useState("");
    const [swapSlotB, setSwapSlotB] = useState("");
    const [history, setHistory] = useState([]);
    const [redoStack, setRedoStack] = useState([]);

    // Update stats whenever timetableData changes
    useEffect(() => {
        if (timetableData && Array.isArray(timetableData.Summary) && timetableData.Summary.length >= 9) {
            setStats(timetableData.Summary);
        } else {
            setStats(null);
        }
    }, [timetableData]);

    if (!timetableData || Object.keys(timetableData).length === 0) {
        return <p className="p-4 text-red-500">No timetable data received.</p>;
    }

    const handleDownload = async () => {
        console.log("frontend working fine");
        try {
            // Ensure we send a clean object. If Summary is missing, the backend might need a default or just ignore it.
            // We construct a payload that includes what we have.
            const payload = { ...timetableData };

            // If Summary is missing, we can optionally add a dummy one if the backend STRICTLY requires it, 
            // otherwise we send it as is. 
            if (!payload.Summary) {
                payload.Summary = [0, 0, 0, 0, 0, 0, 0, 0, 0]; // Default dummy stats to prevent backend crash
            }

            const response = await axios.post(
                `${BASE_URL}/excel/download-excel`,
                payload,
                {
                    responseType: "blob",
                    headers: { "Content-Type": "application/json" },
                }
            );


            const blob = new Blob([response.data], {
                type: "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet",
            });
            const url = window.URL.createObjectURL(blob);
            const link = document.createElement("a");
            link.href = url;
            link.setAttribute("download", "TimeTable.xlsx");
            document.body.appendChild(link);
            link.click();
            link.remove();
        } catch (err) {
            console.error("Download failed:", err);
            alert("Failed to download Excel. Check console for details.");
        }
    };

    const handleSubjectClick = async (subjectName) => {
        try {
            const cleanName = subjectName.split(" (")[0];
            const response = await axios.post(
                `${BASE_URL}/api/slots`,
                {
                    ...timetableData,
                    swap: { subject: cleanName },
                },
                {
                    headers: { "Content-Type": "application/json" },
                }
            );

            const data = response.data;
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

            let fullName = null;
            for (let d = 0; d < updatedTimetable.subjNames.length; d++) {
                for (let s = 0; s < updatedTimetable.subjNames[d].length; s++) {
                    for (const name of updatedTimetable.subjNames[d][s]) {
                        const baseFromName = name.split(" (")[0];
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

            updatedTimetable.examTT[newDay][newSlot].push(subjectId);
            updatedTimetable.subjNames[newDay][newSlot].push(fullName);

            const response = await axios.post(`${BASE_URL}/api/checker`, updatedTimetable, {
                headers: { "Content-Type": "application/json" },
            });

            saveToHistory(response.data);

            setValidSlots([]);
            setSelectedSubject(null);
            setTimetableData(response.data);
        } catch (err) {
            console.error("Failed to move subject:", err);
            alert("Move failed.");
        }
    };

    const handleSwapAnySlots = async () => {
        try {
            const dayA = Number(swapSlotDayA) - 1;
            const dayB = Number(swapSlotDayB) - 1;
            const slotA = Number(swapSlotA) - 1;
            const slotB = Number(swapSlotB) - 1;

            const updatedTimetable = JSON.parse(JSON.stringify(timetableData));

            if (
                dayA < 0 ||
                dayA >= updatedTimetable.examTT.length ||
                dayB < 0 ||
                dayB >= updatedTimetable.examTT.length ||
                slotA < 0 ||
                slotA >= updatedTimetable.slotsPerDay ||
                slotB < 0 ||
                slotB >= updatedTimetable.slotsPerDay
            ) {
                alert("Invalid day or slot numbers.");
                return;
            }

            const originalState = JSON.parse(JSON.stringify(timetableData));
            saveToHistory(originalState);

            // Swap examTT
            const tempExam = updatedTimetable.examTT[dayA][slotA];
            updatedTimetable.examTT[dayA][slotA] =
                updatedTimetable.examTT[dayB][slotB];
            updatedTimetable.examTT[dayB][slotB] = tempExam;

            // Swap subjNames
            const tempNames = updatedTimetable.subjNames[dayA][slotA];
            updatedTimetable.subjNames[dayA][slotA] =
                updatedTimetable.subjNames[dayB][slotB];
            updatedTimetable.subjNames[dayB][slotB] = tempNames;

            const response = await axios.post(`${BASE_URL}/api/checker`, updatedTimetable, {
                headers: { "Content-Type": "application/json" },
            });

            saveToHistory(response.data);

            setTimetableData(response.data);
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
        setHistory((prev) => [...prev, timetableData]);
        setRedoStack([]);
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
                        {/* Safely map slots based on data available */}
                        {[...Array(timetableData.slotsPerDay || 3)].map((_, slotIdx) => (
                            <th key={slotIdx}>SLOT {slotIdx + 1}</th>
                        ))}
                    </tr>
                </thead>
                <tbody>
                    {timetableData.subjNames && timetableData.subjNames.map((daySlots, dayIdx) => (
                        <tr key={dayIdx}>
                            <td>
                                <strong>DAY {dayIdx + 1}</strong>
                            </td>
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

            {/* Summary Section - Conditionally Rendered */}
            <div style={{ display: "flex", justifyContent: "center", marginTop: "20px" }}>
                <div className="summary-container" style={{ flexDirection: 'column', gap: '20px' }}>

                    {/* Only show stats if they exist in the data */}
                    {stats ? (
                        <>
                            <div>
                                <h3 className="text-xl font-bold mb-2 text-center" style={{ color: '#333' }}>Daily Load (Exams per Day)</h3>
                                <div className="summary-container2" style={{ display: 'flex', flexWrap: 'wrap', justifyContent: 'center', gap: '10px' }}>
                                    <div className="summary-box low">
                                        <p>2 Exams in 1 Day</p>
                                        <p><strong>{stats[0]} students</strong></p>
                                    </div>
                                    <div className="summary-box medium">
                                        <p>3 Exams in 1 Day</p>
                                        <p><strong>{stats[1]} students</strong></p>
                                    </div>
                                    <div className="summary-box high">
                                        <p>4 Exams in 1 Day</p>
                                        <p><strong>{stats[2]} students</strong></p>
                                    </div>
                                </div>
                            </div>

                            <div>
                                <h3 className="text-xl font-bold mb-2 text-center" style={{ color: '#333' }}>Consecutive Load (Total Exams in 2 Days)</h3>
                                <div className="summary-container2" style={{ display: 'flex', flexWrap: 'wrap', justifyContent: 'center', gap: '10px' }}>
                                    <div className="summary-box low">
                                        <p>3 Exams / 2 Days</p>
                                        <p><strong>{stats[3]} students</strong></p>
                                    </div>
                                    <div className="summary-box low">
                                        <p>4 Exams / 2 Days</p>
                                        <p><strong>{stats[4]} students</strong></p>
                                    </div>
                                    <div className="summary-box medium">
                                        <p>5 Exams / 2 Days</p>
                                        <p><strong>{stats[5]} students</strong></p>
                                    </div>
                                    <div className="summary-box medium">
                                        <p>6 Exams / 2 Days</p>
                                        <p><strong>{stats[6]} students</strong></p>
                                    </div>
                                    <div className="summary-box high">
                                        <p>7 Exams / 2 Days</p>
                                        <p><strong>{stats[7]} students</strong></p>
                                    </div>
                                    <div className="summary-box high">
                                        <p>8 Exams / 2 Days</p>
                                        <p><strong>{stats[8]} students</strong></p>
                                    </div>
                                </div>
                            </div>
                        </>
                    ) : (
                        <div className="text-gray-500 text-center p-4">

                        </div>
                    )}

                    {/* Valid Slots for Selected Subject - Always render if selected */}
                    {validSlots && (
                        <div className="summary-box" style={{ marginTop: "20px", width: '100%' }}>
                            <p>
                                <strong>Valid slots for "{selectedSubject}":</strong>
                            </p>
                            <ul style={{ padding: 0, listStyle: "none" }}>
                                {validSlots.map(([day, slot], idx) => (
                                    <li
                                        key={idx}
                                        style={{
                                            marginBottom: "8px",
                                            display: "flex",
                                            justifyContent: "space-between",
                                            alignItems: "center",
                                            gap: "10px",
                                        }}
                                    >
                                        <span>
                                            Day {day + 1}, Slot {slot + 1}
                                        </span>
                                        <button
                                            onClick={() => handleMoveSubject(day, slot)}
                                            style={{
                                                padding: "4px 10px",
                                                backgroundColor: "#4CAF50",
                                                color: "white",
                                                border: "none",
                                                borderRadius: "4px",
                                                cursor: "pointer",
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
            </div>

            {/* Swap Section */}
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

            <button
                className="undo-button"
                onClick={handleUndo}
                disabled={history.length === 0}
            >
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

            <div style={{ display: "flex", justifyContent: "center" }}>
                <button className="download-btn" onClick={handleDownload}>
                    Download Excel
                </button>
            </div>
        </div>
    );
};

export default TimetablePage;