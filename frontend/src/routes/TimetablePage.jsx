import { useLocation } from "react-router-dom";
import React from "react";
import "../styles/TimetablePage.css"; // Make sure this import exists

const TimetablePage = () => {
    const location = useLocation();
    const timetableData = location.state?.timetableData;

    if (!timetableData || Object.keys(timetableData).length === 0) {
        return <p className="p-4 text-red-500">No timetable data received.</p>;
    }

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
        </div>
    );
};

export default TimetablePage;
