t <html><head><title>RTC</title>
t <script language=JavaScript type="text/javascript" src="xml_http.js"></script>
t <script language=JavaScript type="text/javascript">
# Define URL and refresh timeout
t var formUpdate = new periodicObj("rtc.cgx", 500);
t function plotRTCGraph() {
t  timeVal = document.getElementById("time").value;
t  dateVal = document.getElementById("date").value;
t  document.getElementById("time").value = (timeVal);
t  document.getElementById("date").value = (dateVal);
t  document.getElementById("rtc_table").style.width = (tableSize + '%');
t  document.getElementById("rtc_volts").value = (voltsVal.toFixed(3) + ' V');
t }
t function periodicUpdateRTC() {
t  if(document.getElementById("rtcChkBox").checked == true) {
t   updateMultiple(formUpdate,plotRTCGraph);
t   rtc_elTime = setTimeout(periodicUpdateRTC, formUpdate.period);
t  }
t  else
t   clearTimeout(rtc_elTime);
t }
t </script></head>
i pg_header.inc
t <h2 align="center"><br>RTC</h2>
t <p><font size="2">This page shows you the hour and date in the server web 
t It also allows you to configure this parameters.</font></p>
t <form action="rtc.cgi" method="post" name="rtc">
t <input type="hidden" value="rtc" name="rtc">
t <table border=0 width=99%><font size="3">
t <tr style="background-color: #aaccff">
t  <th width=15%>Hora</th>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c h 1  size="10" id="time" value="%s"></td></tr>
t  <th width=15%>Fecha</th>
t <td align="center"><input type="text" readonly style="background-color: transparent; border: 0px"
c h 2  size="10" id="date" value="%s"></td>
t </font></table>
t <p align=center>
t <input type=button value="Refresh" onclick="updateMultiple(formUpdate,plotRTCGraph)">
t Periodic:<input type="checkbox" id="rtcChkBox" onclick="periodicUpdateRTC()">
t </p></form>
i pg_footer.inc
. End of script must be closed with period.
