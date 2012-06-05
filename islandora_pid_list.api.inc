<?php
class Islandora_Pid_List_Helper {
  
  public function get_user_pid_list($user) {
    module_load_include('inc', 'islandora_pid_list', 'pidlist');
   
    $listids = array();
    $pidlist = array();
    
    $result = db_query('SELECT listid FROM {islandora_pid_list_lists} WHERE uid = %d', $user);
    
    while ($listid = db_fetch_array($result)) {
    array_push($listids, $listid['listid']);
    }
   
    foreach ($listids as $key => $value) {
    $listpids = array();
    $listusers = array();
    
    $result = db_query('SELECT pidid from {islandora_pid_list_pids} WHERE listid = %d', $value);
    
    while ($pid = db_fetch_array($result)) {
    array_push($listpids, $pid['pidid']);
    }
            
    $listuser = db_query('SELECT uid from {islandora_pid_list_lists} WHERE listid = %d', $value);
    while ($user = db_fetch_array($listuser)) {
      array_push($listusers, $user['uid']);
    }
    
    $listname = db_query('SELECT listname from {islandora_pid_list_names} WHERE listid = %d', $value);
    $listname = db_fetch_array($listname);
    $listid = $value;
    $templist = new PidList(TRUE, $listusers, $listpids, $listname['listname'], $listid);
    
    array_push($pidlist, $templist);
    }
    return $pidlist;
  }
  
  
 



}