<?php

/**
 * @file
 * Definition of PidList.
 *  
 */

/**
 * Class that defines a PidList object. A PidList object needs only to be 
 * constructed such that it contains a listname and listid for uniqueness.
 *   
 */
class PidList {
  /**
   * The list of users who belong to this object.
   * 
   * @var array 
   */
  public $userlist = array();
  
  /**
   * The list of pids that belong to this object.
   *
   * @var array
   */
  public $pidlist = array();
  
  /**
   * The name of the object.
   * 
   * @var string
   */
  public $listname;
  
  /**
   * The unique ID of the object. This is generated through the database or
   * explicity assigned when adding objects to the session.
   * 
   * @var integer
   */
  public $listid; 
  
  /**
   * The owner of the object.
   * 
   * @var integer
   */
  public $listowner;
  
  /**
   * The user id of the current user accessing the object.
   * 
   * @var integer 
   */
  public $useruid;
  
  /**
   * Whether the object is created or retrieved through the session.
   * 
   * @var boolean
   */
  public $session = FALSE;
  
  /**
   * Constructor for the PidList object.
   * 
   * @param array $params 
   */
  public function __construct($params = array()) {
    $this->userlist = $params['listusers'];
    $this->useruid = $params['useruid'];  
    if (!empty($params['session'])) {
      $this->session = $params['session'];
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
  
  /**
   * Removes the specified pids from the object and either 
   * the database or session.
   * 
   * @param array $pids 
   *   An array of pids.
   */
  public function remove_pids($pids) {
    $this->pidlist = array_diff($this->pidlist, $pids);

    if (!$this->session) {
      if ($this->management_access()) {
        foreach ($pids as $value) {
          $this->remove_pid_db($value);
        }
      }
      else {
        throw new Exception(t("You do not have access to remove pids from the list $this->listname"));
      }
    } 
    else {
      $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);

      foreach ($superserial->pidlist as $key => $value) {
        if (in_array($value, $pids)) {
          unset($superserial->pidlist[$key]);
          $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
        }
      }
    }
  }
 
  /**
   * Removes the specified users from the object and either the database
   * or session.
   * 
   * @global type $user
   * 
   * @param array $users 
   *   An array of users.
   */
  public function remove_users($users) {
    global $user;

    $this->userlist = array_diff($this->userlist, $users);
    if (!$this->session) {
      if ($this->management_access()) {
        foreach ($users as $value) {
          $this->remove_user_db($value);
        }
        if (in_array($this->listowner, $users)) {
          db_query("UPDATE {islandora_pid_list_names} SET listowner = NULL WHERE listid = %d", $this->listid);
        }
      }
      elseif (in_array($this->useruid, $users)) {
        $this->remove_user_db($this->useruid);
      }
      else {
         throw new Exception(t("You do not have access to remove users from the list $this->listname"));
      }
    }
    else {
      $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);

      foreach ($superserial->userlist as $key => $value) {
        if (in_array($value, $users)) {
          unset($superserial->userlist[$key]);
          $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
        }
      }
    }
  }
 
  /**
   * Adds the specified pids to the object and either the database or session.
   * 
   * @param array $pids
   *   An array of pids.
   * 
   * @throws Exception 
   * 
   */
  public function add_pids($pids) {
    module_load_include('inc', 'fedora_repository', 'api/tuque');

    foreach ($pids as $key => $value) {
      if (islandora_object_load($value)) {
          if (!in_array($value, $this->pidlist)) {
          array_push($this->pidlist, $value);

            if (!$this->session) {
              if ($this->management_access()) {
                $this->add_pid_db($value);
              }
              else {
                throw new Exception(t("You do not have access to add pids to the list $this->listname"));
              }
            }
            else {
            $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
            $superserial->pidlist[] = $value;     
            $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
            }
          }
      }
      else {
        throw new Exception(t("Could not add the pid '" . $value ."' to '". $this->listname . "' as the pid doesn't exist!"));
      }
    }  
  }
  
  /**
   * Adds the specified users to the object and either the database or session.
   * 
   * @param type $users 
   *   An array of users.
   */
  public function add_users($users) {
    foreach ($users as $key => $value) {
      if (!in_array($value, $this->userlist)) {
        array_push($this->userlist, $value);

          if (!$this->session) {
            if ($this->management_access()) {
              $this->add_user_db($value);
            }
            else {
              throw new Exception(t("You do not have access to add users to the list $this->listname"));
            }
          }
          else {
            $superserial = unserialize($_SESSION['islandora_pid_list'][$this->listid]);
            $superserial->userlist[] = $value;  
            $_SESSION['islandora_pid_list'][$this->listid] = serialize($superserial);
        }
      }
    }
  }
  
  /**
   * Inserts a record into the database or adds the object to the session.
   */
  public function create_list() {
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
      $this->listid = 'session_' . $index;
      $_SESSION['islandora_pid_list'][$this->listid] = serialize($this);
    }
  }

  /**
   * Removes the record from the database or the object from the session.
   */
  public function remove_list() {
    if (!$this->session) {
      if ($this->management_access()) {
      db_query("DELETE FROM {islandora_pid_list_names} WHERE listid = %d", $this->listid);

      foreach ($this->userlist as $key => $value) {
        db_query("DELETE FROM {islandora_pid_list_lists} WHERE listid = %d", $this->listid);
        }

      foreach ($this->pidlist as $key => $value) {
        db_query("DELETE FROM {islandora_pid_list_pids} WHERE listid = %d", $this->listid);
        }
      }
      else {
        throw new Exception(t("You do not have access to remove the list $this->listname"));
      }
    }
    else {
      unset($_SESSION['islandora_pid_list'][$this->listid]);
    } 
  }
  
  /**
   * Removes the specified user from the object's entry in the database.
   * 
   * @param integer $userdelete 
   *   The user to be removed.
   */
  private function remove_user_db($userdelete) {
    db_query("DELETE FROM {islandora_pid_list_lists} WHERE uid = %d AND listid = %d", $userdelete, $this->listid);
  }
  
  /**
   * Adds the specified user to the object's entry in the database.
   * 
   * @param integer $useradd
   *   The user to be added. 
   */
  private function add_user_db($useradd) {
    db_query("INSERT INTO {islandora_pid_list_lists} (uid, listid) VALUES (%d,%d)", $useradd, $this->listid);
  }
  
  /**
   * Removes the specified pid from the object's entry in the database.
   * 
   * @param string $pid
   *   The pid to be removed.
   */
  private function remove_pid_db($pid) {
    db_query("DELETE FROM {islandora_pid_list_pids} WHERE pidid = '%s' AND listid = %d", $pid, $this->listid);
  }
  
  /**
   * Add the specified pid to the object's entry in the database.
   * 
   * @param string $pid 
   *   The user to be added.
   */
  private function add_pid_db($pid) {
    db_query("INSERT INTO {islandora_pid_list_pids} (listid, pidid) VALUES (%d, '%s')", $this->listid, $pid);
  }
 
  /**
   * Changes the name of the object in the session or updates the object's 
   * entry in the database
   * 
   * @param string $name
   *   The new name of the object. 
   */
  public function change_list_name($name) {
    $this->listname = $name;
    if (!$this->session) {
      db_query("UPDATE {islandora_pid_list_names} SET listname = '%s' WHERE listid = %d", $name, $this->listid);
    }
    else {
      $_SESSION['islandora_pid_list'][$this->listid] = serialize($this);
    }
  }
  
  public function management_access() {
    if ($this->useruid === $this->listowner || user_access("Manage other users' pidlists")) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }
}


