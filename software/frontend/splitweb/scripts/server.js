import express from 'express'

// Or use require if nodejs complains about ES module
// const express = require('express')
// const { createServer: createViteServer } = require('vite')

async function createServer() {
    const app = express()

    app.use('/', express.static('./build_shortened'))

    app.listen(3000, function (error) {
        if (error) throw error
        console.log("Static build on http://localhost:3000/")
    })
}

createServer()
