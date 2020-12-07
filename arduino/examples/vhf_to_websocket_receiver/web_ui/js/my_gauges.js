var gauge_elements = [
  document.getElementById("freq_indicator"), 
  document.getElementById("vol_gauge"),
  document.getElementById("rssi_gauge")
];


// https://rockiot-gauge.firebaseapp.com/

var freq_indicator_style = {
"serial":"linear_1",
"type":"gauge",
"variation":"linear",
"orientation":"horizontal",
"name":"Frequency",
"value":"144",
"units":"Hz",
"min":"134",
"max":"174",
"precision":"3",
"animation":"500",
"svgwidth":"300",
"svgheight":"300",
"text-color":"#bababa",
"value-color":"#777",
"value-bg":"transparent",
"value-border":"0px solid #fac83c",
"control-color":"#888",
"control-bg":"none",
"auto-test":1,
"size":"md",
"scale":"3",
"smallscale":"2",
"ticks":"10",
"needle":"1",
"bar-color":"#111",
"progress-color":"#0000",
"scale-color":"#aaa",
"scale-text-color":"#bababa",
"needle-color":"#f80A",
"needle-stroke":"#000",
"zones":"#600,#070"
}

var vol_gauge_style = {
"serial":"linear_3",
"type":"gauge",
"variation":"linear",
"orientation":"vertical",
"name":"Vertical 1",
"value":"40",
"units":"%",
"min":"0",
"max":"100",
"precision":"0",
"animation":"500",
"svgwidth":"300",
"svgheight":"300",
"text-color":"#333",
"value-color":"#777",
"value-bg":"transparent",
"value-border":"0px solid #fac83c",
"control-color":"#888",
"control-bg":"none",
"auto-test":0,
"size":"md",
"scale":"1",
"smallscale":"1",
"ticks":"10",
"needle":"1",
"bar-color":"#111", 
"progress-color":"#69D",  // FILL COLOR
"scale-color":"#aaa",
"scale-text-color":"#333",
"needle-color":"#f40",
"needle-stroke":"#f80",
"zones":"#900,#970,#970,#070,#070,#070,#070,#070"
}

var rssi_gauge_style = {
"serial":"radial_1",
"type":"gauge",
"variation":"radial",
"orientation":"vertical",
"name":"Rx Power",
"value":"2",
"units":"dB",
"min":"0",
"max":"100",
"precision":"0",
"animation":"500",
"svgwidth":"300",
"svgheight":"300",
"text-color":"#bababa",
"value-color":"#777",
"value-bg":"transparent",
"value-border":"0px solid #fac83c",
"control-color":"#888",
"control-bg":"none",
"auto-test":0,
"startangle":"20",
"endangle":"160",
"radius":"sm",
"size":"sm",
"scale":"1",
"smallscale":"1",
"ticks":"5",
"needle":"1",
"bar-color":"#00000060",
"progress-color":"#0000",
"scale-color":"#aaa",
"scale-text-color":"#bababa",
"needle-color":"#f80",
"needle-stroke":"#000"
}

gauge_attributes = [
  freq_indicator_style, 
  vol_gauge_style, 
  rssi_gauge_style
];


// Fix ham band indicator
setAttributesWhenReady("#zones-1-linear_1", {"x":"102", "width":"36"});
setAttributesWhenReady("#zones-0-linear_1", {"x":"20", "width":"330"});

// Adust the freq indicator needle
setAttributesWhenReady("#needle-linear_1", {"width":"3", "y":"38"});
//setAttributesWhenReady("#needle-linear_3", {"height":"3", "transition":"y 0.02s linear 0s" });
setAttributesWhenReady("#fill-linear_3", {"style":"transition: y 0.1s linear 0s" });
setAttributesWhenReady("#needle-linear_3", {"height":"3", "style":"transition: y 0.4s linear 0s" });




// Helper to set attributes for each element:
gauge_elements.forEach(function(element, index){
  console.log(element);
  setAttributes(element, gauge_attributes[index]);
})

// Helper to set attributes individually:
function setAttributes(elem, attrs) {
  //console.log(elem);
  for(var key in attrs) {
    elem.setAttribute(key, attrs[key]);
  }
}

// Helper to wait for element to load then set attributes:
function setAttributesWhenReady(selector, attribs){
  waitForElementToDisplay(selector, function(){
    setAttributes(document.querySelector(selector), attribs);
  },1,4000);
}


// Helper to wait for the gauges to finish loading:
function waitForElementToDisplay(selector, callback, checkFrequencyInMs, timeoutInMs) {
  var startTimeInMs = Date.now();
  console.log("waiting for " + selector);
  (function loopSearch() {
    if (document.querySelector(selector) != null) {
      console.log("found " + selector);
      callback();
      return;
    }
    else {
      setTimeout(function () {
        if (timeoutInMs && Date.now() - startTimeInMs > timeoutInMs){
          console.log("timed out looking for " + selector);
          return;
        }
        loopSearch();
      }, checkFrequencyInMs);
    }
  })();
}
