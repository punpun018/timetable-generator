const express = require('express');
const router = express.Router();
const runCpp3 = require('../utils/runCpp3');

router.post('/', async (req, res) => {
    try {
        const result = await runCpp3(req.body);
        res.json(result);
    } catch (error) {
        res.status(500).json({ error: 'Slot suggestion failed' });
    }
});

module.exports = router;
