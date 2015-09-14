Pebble.addEventListener("showConfiguration", function(e) {
    //Load the remote config page
    Pebble.openURL("http://kristof.willen.be/pebble/dashboard/config.html");
  }
);


Pebble.addEventListener("webviewclosed",
  function(e) {
    //Get JSON dictionary
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("[DBUG] Configuration window returned: " + JSON.stringify(configuration));
 
    //Send to Pebble, persist there
    Pebble.sendAppMessage(
      {'KEY_INVERT': configuration.KEY_INVERT},
      function(e) {
        console.log("[DBUG] Settings data sent successfully ");
      },
      function(e) {
        console.log("[FAIL] Settings feedback failed!");
      }
    );
  }
);