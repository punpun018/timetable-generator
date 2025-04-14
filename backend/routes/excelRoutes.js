const express = require("express");
const ExcelJS = require("exceljs");

const router = express.Router();

router.post("/generate-excel", async (req, res) => {
    try {
        const jsonData = req.body;

        if (!jsonData || !jsonData.timeTable || !jsonData.subjNames || !jsonData.slotsPerDay || !jsonData.days) {
            return res.status(400).json({ error: "Invalid JSON format" });
        }

        const workbook = new ExcelJS.Workbook();

        // ===== First Sheet: Timetable =====
        const worksheet1 = workbook.addWorksheet("Timetable");

        // Header Row
        const headerRow = ['Day'];
        for (let i = 1; i <= jsonData.slotsPerDay; i++) {
            headerRow.push(`Slot ${i}`);
        }
        worksheet1.addRow(headerRow);

        // Bold and center headers
        worksheet1.getRow(1).eachCell(cell => {
            cell.font = { bold: true };
            cell.alignment = { vertical: 'middle', horizontal: 'center' };
        });

        // Fill in timetable data
        for (let day = 0; day < jsonData.days; day++) {
            const row = [`Day ${day + 1}`];
            for (let slot = 0; slot < jsonData.slotsPerDay; slot++) {
                const subjects = jsonData.subjNames[day][slot] || [];
                row.push(subjects.join('\n'));
            }
            worksheet1.addRow(row);
        }

        // Word wrap and alignment
        worksheet1.eachRow(row => {
            row.eachCell(cell => {
                cell.alignment = { wrapText: true, vertical: 'top', horizontal: 'left' };
            });
        });

        // Auto width
        worksheet1.columns.forEach(column => {
            column.width = 30;
        });

        // ===== Second Sheet: Strength Summary =====
        const worksheet2 = workbook.addWorksheet("Strength Summary");

        // Header Row
        worksheet2.addRow(['Day', 'Slot 1', 'Slot 2']);
        worksheet2.getRow(1).eachCell(cell => {
            cell.font = { bold: true };
            cell.alignment = { vertical: 'middle', horizontal: 'center' };
        });

        // Fill in strength values from timeTable
        for (let day = 0; day < jsonData.timeTable.length; day++) {
            const dayData = jsonData.timeTable[day];
            const strength1 = dayData[0][0]; // First slot strength
            const strength2 = dayData[1][0]; // Second slot strength
            worksheet2.addRow([day + 1, strength1, strength2]);
        }

        // Align all cells center
        worksheet2.eachRow(row => {
            row.eachCell(cell => {
                cell.alignment = { vertical: 'middle', horizontal: 'center' };
            });
        });

        // Auto width
        worksheet2.columns.forEach(column => {
            column.width = 15;
        });


        // ===== Third Sheet: Student Exam Summary =====
        const worksheet3 = workbook.addWorksheet("Student Exam Summary");

        // Header row
        const examHeaderRow = ['Student'];
        for (let i = 1; i <= jsonData.days; i++) {
            examHeaderRow.push(`Day ${i}`);
        }
        worksheet3.addRow(examHeaderRow);

        // Bold and center headers
        worksheet3.getRow(1).eachCell(cell => {
            cell.font = { bold: true };
            cell.alignment = { vertical: 'middle', horizontal: 'center' };
        });

        // Fill in each student's exam data
        jsonData.timeTableSummary.forEach(student => {
            const row = [student.rollNumber, ...student.exams];
            worksheet3.addRow(row);
        });

        // Align all cells
        worksheet3.eachRow(row => {
            row.eachCell(cell => {
                cell.alignment = { vertical: 'middle', horizontal: 'center' };
            });
        });

        // Auto width
        worksheet3.columns.forEach(column => {
            column.width = 15;
        });
        // ===== Fourth Sheet: Student Summary 2 (Subject Names) =====
        const worksheet4 = workbook.addWorksheet("Student Summary 2");

        // Header row
        const subjHeaderRow = ['Student'];
        for (let i = 1; i <= jsonData.days; i++) {
            subjHeaderRow.push(`Day ${i}`);
        }
        worksheet4.addRow(subjHeaderRow);

        // Bold and center headers
        worksheet4.getRow(1).eachCell(cell => {
            cell.font = { bold: true };
            cell.alignment = { vertical: 'middle', horizontal: 'center' };
        });

        // Fill in subject names from each student's 'subs' array
        jsonData.timeTableSummary.forEach(student => {
            const subjectsPerDay = student.subs.map(daySubs => daySubs.join(', '));
            const row = [student.rollNumber, ...subjectsPerDay];
            worksheet4.addRow(row);
        });

        // Align and format cells
        worksheet4.eachRow(row => {
            row.eachCell(cell => {
                cell.alignment = { wrapText: true, vertical: 'middle', horizontal: 'center' };
            });
        });

        // Auto width
        worksheet4.columns.forEach(column => {
            column.width = 20;
        });

        // Send file
        res.setHeader("Content-Type", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
        res.setHeader("Content-Disposition", "attachment; filename=timetable.xlsx");

        await workbook.xlsx.write(res);
        res.end();
    } catch (error) {
        console.error("Error creating Excel:", error);
        res.status(500).json({ error: "Failed to generate Excel" });
    }
});

module.exports = router;
