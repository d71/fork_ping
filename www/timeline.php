<?php

if (isset($_GET["host"])){

    $host=$_GET["host"];
    
    echo "<b>$host</b><br>";


    $link = mysql_connect("localhost", "ping", "mypingtest")	//db connect
        or die("Could not connect : " . mysql_error());		////////////
    mysql_select_db("ping") or die("Could not select database");	////////////



    $sql = "SELECT * FROM timeline WHERE host='$host' ORDER by time1 DESC LIMIT 144"; 
    $result = mysql_query($sql);

    while ($line = mysql_fetch_array($result, MYSQL_ASSOC)) {

	if ($line[ping]=="1"){
	    echo "<font color=green>$line[time1]</font> ";
	}else{
    	    echo "<font color=red><b>$line[time1]</b></font> ";
	}
    
	echo "<br>";

    }




mysql_free_result($result);
mysql_close($link);


}else{
    echo "host?";
}

?>