<html>
 <head>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
  <title>ping result</title>
  <style type="text/css">
   A {
    text-decoration: none; 
   } 
   A:hover { 
    text-decoration: underline; 
   } 
  </style>
 </head>
 <body> 


<table border=0>
<tr bgcolor=#d0d0d0><td>ip</td><td>description</td><td>timeline</td>

<?php

$link = mysql_connect("localhost", "ping", "mypingtest")	//db connect
        or die("Could not connect : " . mysql_error());		////////////
mysql_select_db("ping") or die("Could not select database");	////////////




    $sql = "SELECT * FROM hosts";
    $result = mysql_query($sql);
    $count=0;

while ($line = mysql_fetch_array($result, MYSQL_ASSOC)) {
    
    $count++;

    if ($line[life]==1){
	$color="green";
    }else{
	$color="red";
    }

    echo "<tr bgcolor=#f0f0f0>
	<td><a href=timeline.php?host=$line[ip]><font color=$color><b>$line[ip]</b></font></a></td>
	<td>$line[description]</td>
	<td><a href=timeline.php?host=$line[ip]><img src=graph/$line[ip].png></a></td>
	";
    
}

echo "</table><br>
	total: $count
    ";

mysql_free_result($result);
mysql_close($link);

?>

 </body>
</html>