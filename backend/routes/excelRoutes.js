const express = require("express");
const ExcelJS = require("exceljs");
const router = express.Router();

router.post("/download-excel", async (req, res) => {

    try {
        const { days, timeTable, timeTableSummary } = req.body;

        const workbook = new ExcelJS.Workbook();

        // ====================================================================
        //  FIRST SHEET — Subject Summary
        // ====================================================================
        const sheet = workbook.addWorksheet("Exam Timetable");

        const numSlots = timeTable[0].length;

        // HEADER
        let header = ["Day / Slot"];
        for (let s = 1; s <= numSlots; s++) {
            header.push(`Slot ${s}`);
        }

        const headerRow = sheet.addRow(header);

        // --- HEADER STYLING ---
        headerRow.eachCell(cell => {
            cell.font = { bold: true, color: { argb: "FFFFFFFF" } };   // white text
            cell.fill = {
                type: "pattern",
                pattern: "solid",
                fgColor: { argb: "FF003366" }   // dark blue
            };
            cell.alignment = { vertical: "middle", horizontal: "center", wrapText: true };
        });

        // --- ROWS ---
        // --- ROWS ---
        for (let dayIndex = 0; dayIndex < days; dayIndex++) {
            const rowCells = [`Day ${dayIndex + 1}`];
            let maxLines = 1; // track largest cell in this row

            for (let slotIndex = 0; slotIndex < numSlots; slotIndex++) {
                const [strength, subjects] = timeTable[dayIndex][slotIndex];

                // Join with newlines
                let text = subjects && subjects.length > 0
                    ? subjects.join("\n")
                    : "";

                rowCells.push(text);

                // Count lines in this cell
                const lineCount = text.split("\n").length;
                maxLines = Math.max(maxLines, lineCount);
            }

            const row = sheet.addRow(rowCells);

            // Apply text wrapping & alignment
            row.eachCell(cell => {
                cell.alignment = {
                    wrapText: true,
                    vertical: "top",
                };
            });

            // --- ALTERNATE ROW COLORS ---
            const isEven = (dayIndex + 1) % 2 === 0;
            row.eachCell(cell => {
                cell.fill = {
                    type: "pattern",
                    pattern: "solid",
                    fgColor: { argb: isEven ? "FFF2F2F2" : "FFFFFFFF" }
                };
            });

            // 🎯 AUTO RESIZE ROW HEIGHT BASED ON MAX SUBJECTS
            const heightPerLine = 15; // tweak this if needed
            row.height = maxLines * heightPerLine;
        }

        // Column width
        sheet.columns.forEach(col => col.width = 30);

        // ====================================================================
        //  SECOND SHEET — Strength Summary
        // ====================================================================
        const strengthSheet = workbook.addWorksheet("Strength Summary");

        // ---- HEADER (same as sheet 1) ----
        const strengthHeader = ["Day / Slot"];
        for (let s = 0; s < numSlots; s++) strengthHeader.push(`Slot ${s + 1}`);

        const strengthHeaderRow = strengthSheet.addRow(strengthHeader);

        // Header styling
        strengthHeaderRow.eachCell(cell => {
            cell.fill = {
                type: "pattern",
                pattern: "solid",
                fgColor: { argb: "FF1F4E78" } // Dark blue
            };
            cell.font = { bold: true, color: { argb: "FFFFFFFF" } };
            cell.alignment = { horizontal: "center" };
        });

        // ---- CONTENT ROWS ----
        for (let dayIndex = 0; dayIndex < days; dayIndex++) {
            const rowCells = [`Day ${dayIndex + 1}`];
            let maxLines = 1;

            for (let slotIndex = 0; slotIndex < numSlots; slotIndex++) {
                const [strength, subjects] = timeTable[dayIndex][slotIndex];

                // Only the strength
                const text = String(strength);
                rowCells.push(text);

                maxLines = Math.max(maxLines, 1);
            }

            const row = strengthSheet.addRow(rowCells);

            // Wrap + align
            row.eachCell(cell => {
                cell.alignment = {
                    wrapText: true,
                    vertical: "middle",
                    horizontal: "center"
                };
            });

            // Alternate row color
            const isEven = (dayIndex + 1) % 2 === 0;
            row.eachCell(cell => {
                cell.fill = {
                    type: "pattern",
                    pattern: "solid",
                    fgColor: { argb: isEven ? "FFF2F2F2" : "FFFFFFFF" }
                };
            });

            // Height (fixed since strength is 1 line)
            row.height = 20;
        }

        // Column widths
        strengthSheet.columns.forEach(col => col.width = 20);

        // ---------- SHEET 3: STUDENT EXAM COUNT ----------
        const examSheet = workbook.addWorksheet("Student Exam Count");

        // Extract summary
        const summary = timeTableSummary;  // <-- ensure this path matches your JSON object

        if (!summary || summary.length === 0) {
            console.log("timeTableSummary missing!");
        }

        // Number of days is equal to exams.length
        const numberOfDays = summary[0].exams.length;

        // ----- HEADER -----
        header = ["Roll Number"];
        for (let d = 1; d <= numberOfDays; d++) header.push(`Day ${d}`);
        examSheet.addRow(header);

        // Style header
        header.forEach((_, idx) => {
            examSheet.getRow(1).getCell(idx + 1).fill = {
                type: "pattern",
                pattern: "solid",
                fgColor: { argb: "1F4E78" }, // dark blue
            };
            examSheet.getRow(1).getCell(idx + 1).font = { bold: true, color: { argb: "FFFFFF" } };
            examSheet.getRow(1).getCell(idx + 1).alignment = { vertical: "middle", horizontal: "center" };
        });

        // ----- DATA ROWS -----
        summary.forEach((student, index) => {
            const row = [student.rollNumber, ...student.exams];
            const addedRow = examSheet.addRow(row);

            // Alternate row coloring
            const fillColor = index % 2 === 0 ? "FFFFFF" : "DDEBF7";
            addedRow.eachCell((cell) => {
                cell.fill = {
                    type: "pattern",
                    pattern: "solid",
                    fgColor: { argb: fillColor },
                };
                cell.alignment = { vertical: "middle", horizontal: "center" };
            });
        });

        // ----- AUTO COLUMN WIDTH -----
        examSheet.columns.forEach((col) => {
            let maxLength = 10;
            col.eachCell({ includeEmpty: true }, (cell) => {
                const value = cell.value ? cell.value.toString() : "";
                if (value.length > maxLength) maxLength = value.length;
            });
            col.width = maxLength + 2;
        });

        // ----- AUTO ROW HEIGHT -----
        examSheet.eachRow((row) => {
            let maxLines = 1;
            row.eachCell((cell) => {
                const value = cell.value ? cell.value.toString() : "";
                const lineCount = value.split("\n").length;
                if (lineCount > maxLines) maxLines = lineCount;
            });
            row.height = 15 * maxLines;
        });

        // ---------- SHEET 4: STUDENT SUBJECTS PER DAY ----------
        const subjectSheet = workbook.addWorksheet("Student Subjects");

        // Extract summary
        const summary2 = timeTableSummary;

        if (!summary2 || summary2.length === 0) {
            console.log("timeTableSummary missing!");
        }

        // Days = length of subs array
        const numDays = summary2[0].subs.length;

        // ----- HEADER -----
        const header2 = ["Roll Number"];
        for (let d = 1; d <= numDays; d++) header2.push(`Day ${d}`);
        subjectSheet.addRow(header2);

        // Style header
        header2.forEach((_, idx) => {
            subjectSheet.getRow(1).getCell(idx + 1).fill = {
                type: "pattern",
                pattern: "solid",
                fgColor: { argb: "1F4E78" }, // dark blue
            };
            subjectSheet.getRow(1).getCell(idx + 1).font = { bold: true, color: { argb: "FFFFFF" } };
            subjectSheet.getRow(1).getCell(idx + 1).alignment = { vertical: "middle", horizontal: "center" };
        });

        // ----- DATA ROWS -----
        summary2.forEach((student, index) => {
            // convert subjects to newline-separated strings
            const subjectStrings = student.subs.map((subList) => {
                if (!subList || subList.length === 0) return "";  // empty slot
                return subList.join("\n");  // subject1\nsubject2\n...
            });

            const row = [student.rollNumber, ...subjectStrings];
            const addedRow = subjectSheet.addRow(row);

            // Alternate row coloring
            const fillColor = index % 2 === 0 ? "FFFFFF" : "DDEBF7";
            addedRow.eachCell((cell) => {
                cell.fill = {
                    type: "pattern",
                    pattern: "solid",
                    fgColor: { argb: fillColor },
                };
                cell.alignment = { wrapText: true, vertical: "middle", horizontal: "center" };
            });
        });

        // ----- AUTO COLUMN WIDTH -----
        subjectSheet.columns.forEach((col) => {
            let maxLength = 10;
            col.eachCell({ includeEmpty: true }, (cell) => {
                const value = cell.value ? cell.value.toString() : "";
                value.split("\n").forEach((line) => {
                    if (line.length > maxLength) maxLength = line.length;
                });
            });
            col.width = maxLength + 2;
        });

        // ----- AUTO ROW HEIGHT -----
        subjectSheet.eachRow((row) => {
            let maxLines = 1;
            row.eachCell((cell) => {
                const value = cell.value ? cell.value.toString() : "";
                const lineCount = value.split("\n").length;
                if (lineCount > maxLines) maxLines = lineCount;
            });
            row.height = 15 * maxLines;
        });



        // --- EXPORT ---
        res.setHeader(
            "Content-Type",
            "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"
        );
        res.setHeader(
            "Content-Disposition",
            "attachment; filename=exam_timetable.xlsx"
        );

        await workbook.xlsx.write(res);
        res.end();

    } catch (err) {
        console.error("Excel creation error:", err);
        res.status(500).json({ error: "Failed to generate Excel" });
    }
});

module.exports = router;
