<?php

class PidList {
public $userlist;
public $pidlist;
public $listname;  
public $listid; 

public function __construct($retrieve = FALSE, $users, $pids = NULL, $name = NULL, $id = NULL) {
  $this->userlist = $users;
    
  if (!empty($pids))
    $this->pidlist = $pids;
     
  if (!empty($name))
    $this->listname = $name;
  
  if (!empty($id))
    $this->listid = $id;
 
  if ($retrieve == FALSE) {
    $this->create_list();
   }
}  

public function remove_pids($pids) {
  $this->pidlist = array_diff($this->pidlist, $pids);
  
  foreach ($pids as $value) {
  $this->remove_pid_db($value);
  }
}

public function remove_users($users) {
  $this->userlist = array_diff($this->userlist, $users);
  
  foreach ($pids as $value) {
  $this->remove_user_db($value);
 }
}

public function add_pids($pids) {
  foreach ($pids as $key => $value) {
    if (!in_array($value,$this->pidlist)) {
    array_push($this->pidlist, $value);
    
    // Session + DB 
    $this->add_pid_db($value);
    }
  }  
}
public function add_users($users) {
  foreach ($users as $key => $value) {
    if (!in_array($value,$this->userlist)) {
    array_push($this->userlist, $value);
    
    // Session + DB CONDITION
    $this->add_user_db($value);
    }
  }
}

private function create_list() {
   // Session + DB CONDITION
   db_query("INSERT INTO {islandora_pid_list_names} (listname) VALUES ('%s')", $this->listname);
   
   // Use db_next_id for D7
   $this->listid = db_last_insert_id('{islandora_pid_list_names}', 'listid');
        
   foreach ($this->userlist as $key => $value) {
     db_query("INSERT INTO {islandora_pid_list_lists} (uid, listid) VALUES (%d,%d)", $value, $this->listid);
   }
   
   foreach ($this->pidlist as $key => $value) {
     db_query("INSERT INTO {islandora_pid_list_pids} (listid, pidid) VALUES (%d,'%s')", $this->listid, $value);
   }
  }
 
public function remove_list() {
  // Session + DB CONDITION
  db_query("DELETE FROM {islandora_pid_list_names} WHERE listid = %d", $this->listid);
  
  foreach ($this->userlist as $key => $value) {
    db_query("DELETE FROM {islandora_pid_list_lists} WHERE listid = %d", $this->listid);
  }
  
  foreach ($this->pidlist as $key => $value) {
    db_query("DELETE FROM {islandora_pid_list_pids} WHERE listid = %d", $this->listid);
  }
  }
  
private function remove_user_db($user) {
db_query("DELETE FROM {islandora_pid_list_lists} WHERE uid = %d AND listid = %d", $user, $this->listid);
}

private function add_user_db($user) {
db_query("INSERT INTO {islandora_pid_list_lists} (uid, listid) VALUES (%d,%d)", $user, $this->listid);
}

private function remove_pids_db($pids) {
  foreach ($pids as $key => $value) {
    db_query("DELETE FROM {islandora_pid_list_pids} WHERE pidid = '%s'", $value);
  }
}

private function add_pids_db($pids) {
  foreach ($pids as $key => $value) {
    db_query("INSERT INTO {islandora_pid_list_pids} (listid, pidid) VALUES (%d, '%s')", $this->listid, $value);
  }
}

public function change_list_name($name) {
  db_query("UPDATE {islandora_pid_list_names} SET listname = '%s' WHERE listid = %d", $name, $this->listid);
}

public function arr_exist($array, $val) {
  foreach ($array as $item => $value)
  {
    if ($value == $val) {
      return TRUE;
   }
  }
  return FALSE;
}

}


