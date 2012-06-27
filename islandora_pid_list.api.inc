<?php
module_load_include('inc', 'islandora_pid_list', 'pidlist');

function get_user_pid_list($params = array()) {
  global $user;
  $listids = array();
  $pidlist = array();
    
  if (array_key_exists('user', $params)) {
    $uid = $params['user'];
  }
    
  if (array_key_exists('pid', $params)) {
    $pid = $params['pid'];
  }
  
  if (array_key_exists('all', $params)) {
    $alllists = $params['all'];
  }
  
  
  if (isset($pid)) {
    $result = db_query('SELECT listid FROM {islandora_pid_list_pids} WHERE pidid = "%s" ORDER BY listid', $pid);
  }
  elseif (isset($uid)) {
    $result = db_query('SELECT listid FROM {islandora_pid_list_lists} WHERE uid = %d ORDER BY listid', $uid);
  }
  else {
    $result = db_query('SELECT DISTINCT listid FROM {islandora_pid_list_lists} WHERE uid <> %d ORDER BY listid', $user->uid);
    $orphanresult = db_query('SELECT listid from {islandora_pid_list_names} WHERE listowner IS NULL');
    while ($orphanid = db_fetch_array($orphanresult)) {
      array_push($listids, $orphanid['listid']);
    }
  }
 
  while ($listid = db_fetch_array($result)) {
    if (!in_array($listid['listid'], $listids)) {
      array_push($listids, $listid['listid']);
    }
  }
  
  foreach ($listids as $key => $value) {
    $listpids = array();
    $listusers = array();
    
    $result = db_query('SELECT pidid FROM {islandora_pid_list_pids} WHERE listid = %d', $value);

    while ($pids = db_fetch_array($result)) {
      array_push($listpids, $pids['pidid']);
    }
             
    $listuser = db_query('SELECT uid FROM {islandora_pid_list_lists} WHERE listid = %d', $value);
              
    while ($users = db_fetch_array($listuser)) {
      array_push($listusers, $users['uid']);
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
   if ($user->uid == 0 && !empty($_SESSION['islandora_pid_list'])) {
     foreach ($_SESSION['islandora_pid_list'] as $value) {
       if (isset($pid)) {
         $templist = unserialize($value);
           if (in_array($pid, $templist->pidlist)) {
             $pidlist[] = $templist;
           }
       }
       else {
          $pidlist[] = unserialize($value);
        }
     }
   }
  return $pidlist;
}

function get_pid_list_by_number($listid) {
  dsm($_SESSION['islandora_pid_list'], 'session');
  dsm($listid, 'listid');
  
  if (!preg_match('/session_/', $listid)) {
    
    module_load_include('inc', 'islandora_pid_list', 'pidlist');
    $listresult = db_query('SELECT listname, listowner from {islandora_pid_list_names} WHERE listid = %d', $listid);

    $params = array(
      'retrieve' => TRUE,
      'useruid' => $uid,
      'listowner' => $listresult['listowner'],
      'listname' => $listresult['listname'],
      'listid' => $listid,
    );
    $templist = new PidList($params);
  }
  else {
    $templist = unserialize($_SESSION['islandora_pid_list'][$listid]);
  }
  return $templist;
}