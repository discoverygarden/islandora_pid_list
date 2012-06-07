<?php

class PidList {
public $userlist;
public $pidlist;
public $listname;  
public $listid; 
public $listowner;
public $useruid;
public $session = FALSE;

public function __construct($params = array()) {
  $this->userlist = $params['listusers'];
  $this->useruid = $params['useruid'];  
  
  if ($this->useruid == 0) {
  $this->session = TRUE;
  }
  
  if (!empty($params['listpids'])) {
    $this->pidlist = $params['listpids'];
  }
  
  if (!empty($params['listname'])) {
    $this->listname = $params['listname'];
  }
  
  if (!empty($params['listid'])) {
    $this->listid = $params['listid'];
  }
  
  if (!$params['retrieve']) {
    $this->create_list();
   }
  else {
    $this->listowner = $params['listowner'];
  } 
}  

public function remove_pids($pids) {
  $this->pidlist = array_diff($this->pidlist, $pids);
  
  if (!$this->session) {
    foreach ($pids as $value) {
      $this->remove_pid_db($value);
    }
  } 
  else {
    $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
     
    foreach ($superserial->pidlist as $key => $value) {
      if (in_array($value,$pids)) {
        unset($superserial->pidlist[$key]);
        $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
      }
    }
  }
}

public function remove_users($users) {
  $this->userlist = array_diff($this->userlist, $users);
  
  if (!$this->session) {
    foreach ($users as $value) {
      if ($this->useruid == $this->listowner) {
        $this->remove_user_db($value);
      }
      else {
        dsm("You aren't the owner of this list");
        // Meaningful error replaces
      }
    }
  }
  else {
    $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
     
    foreach ($superserial->userlist as $key => $value) {
     if (in_array($value,$users)) {
      unset($superserial->userlist[$key]);
      $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
     }
    }
  }
}

public function add_pids($pids) {
  foreach ($pids as $key => $value) {
    if (!in_array($value,$this->pidlist)) {
      array_push($this->pidlist, $value);
    
      if (!$this->session) {
        $this->add_pid_db($value);
      }
      else {
          $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
          $superserial->pidlist[] = $value;     
      }
    }
  }  
}
public function add_users($users) {
  foreach ($users as $key => $value) {
      if (!in_array($value,$this->userlist)) {
      array_push($this->userlist, $value);
    
        if (!$this->session) {
          if ($this->useruid == $this->listowner) {
            $this->add_user_db($value);
          }
          else {
            dsm("You aren't the owner of this list");
            // Meaningful error
          }
        }
        else {
          $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
          $superserial->userlist[] = $value;     
       }
     }
   }
}

public function create_list() {
  dsm("Create list called");
  $this->listowner = $this->useruid;
  
  if (!$this->session) {
   db_query("INSERT INTO {islandora_pid_list_names} (listname, listowner) VALUES ('%s',%d)", $this->listname, $this->useruid);
   $this->listid = db_last_insert_id('{islandora_pid_list_names}', 'listid');
   
     foreach ($this->userlist as $key => $value) {
       db_query("INSERT INTO {islandora_pid_list_lists} (uid, listid) VALUES (%d,%d)", $value, $this->listid);
     }
     foreach ($this->pidlist as $key => $value) {
       db_query("INSERT INTO {islandora_pid_list_pids} (listid, pidid) VALUES (%d,'%s')", $this->listid, $value);
     }
  }
  else {
    // Add to session
    $index = count($_SESSION['islandora_pid_list']);
    $this->listid = $index;
    $_SESSION['islandora_pid_list'][$index] = serialize($this);
   }
}

 
public function remove_list() {
  if (!$this->session) {
    if ($this->useruid == $this->listowner) {
      db_query("DELETE FROM {islandora_pid_list_names} WHERE listid = %d", $this->listid);
  
        foreach ($this->userlist as $key => $value) {
          db_query("DELETE FROM {islandora_pid_list_lists} WHERE listid = %d", $this->listid);
        }
  
        foreach ($this->pidlist as $key => $value) {
          db_query("DELETE FROM {islandora_pid_list_pids} WHERE listid = %d", $this->listid);
        }
      }
    else {
      dsm("You aren't the owner of this list");
      // Meaningful error
    }
  }
  else {
   dsm($_SESSION['islandora_pid_list'], 'before unset');
   unset($_SESSION['islandora_pid_list'][$this->listid]);
   dsm($_SESSION['islandora_pid_list'], "after unset");
  } 
}
  
private function remove_user_db($userdelete) {
  db_query("DELETE FROM {islandora_pid_list_lists} WHERE uid = %d AND listid = %d", $userdelete, $this->listid);
}

private function add_user_db($useradd) {
  db_query("INSERT INTO {islandora_pid_list_lists} (uid, listid) VALUES (%d,%d)", $useradd, $this->listid);
}

private function remove_pid_db($pids) {
  foreach ($pids as $key => $value) {
    db_query("DELETE FROM {islandora_pid_list_pids} WHERE pidid = '%s'", $value);
  }
}

private function add_pid_db($pids) {
  foreach ($pids as $key => $value) {
    db_query("INSERT INTO {islandora_pid_list_pids} (listid, pidid) VALUES (%d, '%s')", $this->listid, $value);
  }
}

public function change_list_name($name) {
  $this -> listname = $name;
  if (!$session) {
  db_query("UPDATE {islandora_pid_list_names} SET listname = '%s' WHERE listid = %d", $name, $this->listid);
  }
  else {
  $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
  $superserial->listname[$key][$this->listid][$name];   
  }
  
}

public function set_session($param) {
  if (is_bool($param))
  {
    $this->session = $param;
  }
}

}


