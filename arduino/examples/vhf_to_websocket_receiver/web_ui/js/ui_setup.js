var freqIndicatorTextSpan;

window.onload = (event) => {
  tuner = $('.rotarySwitch').rotaryswitch({
    minimum: 134, 
    maximum: 174, 
    step: 0.1, 
    themeClass: 'big',
    originDeg: 180, // defines the zero position relative to up
    zoom: 0.4
  });
  tuner.on('change',function(){
    document.getElementById("freq_indicator").value = tuner.val();
  })
  
  // Hide the gauge UI config tools
  document.getElementsByClassName('rockiot-ui-control')[2].innerHTML = "";
  
  // Make the frequency indicator text editable, and intercept the return key
  freqIndicatorTextSpan = document.getElementsByClassName("rockiot-gauge-linear-horizontal-value")[0].children[0];
  freqIndicatorTextSpan.contentEditable = true;
  freqIndicatorTextSpan.onkeydown = function(e) {
    // trap the return key being pressed
    if (e.keyCode === 13) {
      e.preventDefault();
      setFreq(freqIndicatorTextSpan.innerText);
    }
  }
  // change frequency when you click out of the freq indicator text box
  freqIndicatorTextSpan.addEventListener("focusout", function(){
    setFreq(freqIndicatorTextSpan.innerText);
  });
}

function setFreq(freq){
  document.getElementById("freq_indicator").value = freq;
  tuner.val(freq);
  console.log(freq);
  document.getElementsByClassName("switch")[0].style.transform = "rotate(" + ((freq-134)/(174-134)*360+180) + "deg)";
}
