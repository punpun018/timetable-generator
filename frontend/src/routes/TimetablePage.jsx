import { useLocation } from "react-router-dom";
import React, { useState } from "react";
import "../styles/TimetablePage.css";

const TimetablePage = () => {
    const location = useLocation();
    const timetableData = location.state?.timetableData;
    // console.log("Summary received:", timetableData.Summary);
    if (!timetableData || Object.keys(timetableData).length === 0) {
        return <p className="p-4 text-red-500">No timetable data received.</p>;
    }

    const handleDownload = async () => {
        // console.log("button used");
        try {
            const response = await fetch("http://localhost:5000/excel/generate-excel", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                },
                body: JSON.stringify(timetableData), // Send the timetable JSON here
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
                                                <span key={idx} className="subject-pill">{subj}</span>
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

            <div style={{ display: "flex", alignItems: "center", width: "100%", justifyContent: "center" }}>
                {Array.isArray(timetableData.Summary) && timetableData.Summary.length === 3 && (
                    <div className="summary-container">
                        <div className="summary-box">
                            <p><strong>Students with exactly 2 exams on a day:</strong> {timetableData.Summary[0]}</p>
                            <p><strong>Students with 3 exams on two consecutive days:</strong> {timetableData.Summary[1]}</p>
                            <p><strong>Students with 4 exams on two consecutive days:</strong> {timetableData.Summary[2]}</p>
                        </div>
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
