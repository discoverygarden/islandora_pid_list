<?php
function get_user_pid_list($uid) {
    module_load_include('inc', 'islandora_pid_list', 'pidlist');
   
    $listids = array();
    $pidlist = array();
    
    $result = db_query('SELECT listid FROM {islandora_pid_list_lists} WHERE uid = %d ORDER BY listid', $uid);
    
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

        $listresult = db_query('SELECT listname, listowner from {islandora_pid_list_names} WHERE listid = %d', $value);
        $listresult = db_fetch_array($listresult);

        $listid = $value;

        $params = array(
          'retrieve' => TRUE,
          'useruid' => $uid,
          'listowner' => $listresult['listowner'],
          'listusers' => $listusers,
          'listpids' => $listpids,
          'listname' => $listresult['listname'],
          'listid' => $listid
        );
        $templist = new PidList($params);

        $pidlist[] = $templist;
      }
         
    // User is anon, use session as well
    if ($user->uid == 0) {
       if (!empty($_SESSION['islandora_pid_list'])) {
          foreach($_SESSION['islandora_pid_list'] as $value) {
            $pidlist[] = unserialize($value);
          }
        }
    }
    return $pidlist;
}

function get_pid_list_pid($pid) {
  
}