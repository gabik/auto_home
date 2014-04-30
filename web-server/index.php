<?php
    if (isset($_POST['power_on']))
    {
				exec(' ./power_on.sh 12 2>&1', $outputAndErrors, $return_value);
				echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "alert(\"$value\");";
				}
				echo '</script>';
    }
    if (isset($_POST['power_off']))
    {
				exec(' ./power_off.sh 12 2>&1', $outputAndErrors, $return_value);
				echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "alert(\"$value\");";
				}
				echo '</script>';
    }
    if (isset($_POST['power_status']))
    {
				exec(' ./power_status.sh 12 2>&1', $outputAndErrors, $return_value);
				echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "alert(\"$value\");";
				}
				echo '</script>';
    }
    if (isset($_POST['power_time']))
    {
				exec(' ./power_time.sh 12 2>&1', $outputAndErrors, $return_value);
				echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "alert(\"$value\");";
				}
				echo '</script>';
    }
    if (isset($_POST['tempreture']))
    {
				exec(' ./tempreture.sh 12 2>&1', $outputAndErrors, $return_value);
				echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "alert(\"$value\");";
				}
				echo '</script>';
    }

    if (isset($_POST['update_status']))
    {
				exec(' ./update_status.sh 12 2>&1', $outputAndErrors, $return_value);
				//echo '<script type="text/javascript">';
				foreach($outputAndErrors as $key => $value) {
					echo "$value <BR>";
				}
				echo '</script>';
    }
?>

<html>
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <title>Home automation - Gabi</title>
</head>

<body>
<form method="post">
<button name="power_on">Power On</button><BR>
<button name="power_off">Power Off</button><BR>
<button name="power_status">Power Status</button><BR>
<button name="power_time">Power Time</button><BR>
<button name="tempreture">Get Tempreture</button><BR>
<button name="update_status">Update Status</button><BR>
<div name=status></div>
<div name=time></div>
<div name=temp></div>
</form>
</body>
</html>
