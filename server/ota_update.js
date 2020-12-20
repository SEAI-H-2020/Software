//BASED on https://hackernoon.com/building-your-own-esp8266esp32-over-the-air-firmware-updater-a-how-to-guide-part-ii-psb53y1q
const path = require('path');
var md5 = require('md5-file');

module.exports = function(app, pool){
    app.get("/update/", async(req, res) => {
        /*
            Swagger Documentation:
        */   
        console.log(req)
        var fwv = req.headers['x-esp32-version'];
        var filePath = path.join(__dirname, './updates/firmwareV'+fwv+'.bin');
        console.log("firmware version: " + fwv);
        console.log("path: " + filePath);

	    var options = {
	    	headers: {
	    		"x-MD5":  md5.sync(filePath)
	    	}
	    }

	    res.sendFile(filePath, function (err) {
	    	if (err) {
                console.log(err);
	    	} else {
	    		console.log('Sent:', filePath);
	    	}
	    })
    });
    
}