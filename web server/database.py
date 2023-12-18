import sqlite3
import hashlib
from datetime import datetime


# Function to create SQLite3 connection
def create_sqlite_connection(name='database.db'):
    return sqlite3.connect(name)


# Initialize SQLite3 database
def init_db(connection):
    cursor = connection.cursor()

    # Create users table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL
        )
    ''')

    # Create warnings table
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS warnings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            description TEXT,
            status INTEGER CHECK (status IN (0, 1)) DEFAULT 0,
            date DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')

    connection.commit()


def add_user_to_db(data):
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        query = '''
            INSERT INTO users(username, password)
            VALUES (?, ?)
        '''
        try:
            cursor.execute(query, (
                data['username'],
                data['password']
            ))
            conn.commit()
        except Exception as e:
            return -1


def is_user_exist_in_db(username=None):
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        if username:
            query = 'SELECT * FROM users WHERE username = ?'
            params = (username,)
        else:
            return False  # Invalid parameters provided

        result = cursor.execute(query, params)
        rows = result.fetchall()
        return len(rows) > 0


def get_user_password_from_db(username=None):
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        if username:
            query = 'SELECT password FROM users WHERE username = ?'
            params = (username,)
        else:
            return None  # Invalid parameters provided

        result = cursor.execute(query, params)
        row = result.fetchone()
        if row:
            return row[0]
        else:
            return None


def get_user_data_from_db(username=None, id=None):
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        if username:
            query = 'SELECT * FROM users WHERE username = ?'
            params = (username,)
        elif id:
            query = 'SELECT * FROM users WHERE id = ?'
            params = (id,)
        else:
            return None  # Invalid parameters provided

        result = cursor.execute(query, params)
        row = result.fetchone()
        if row:
            columns = [column[0] for column in result.description]
            user_dict = dict(zip(columns, row))
            return user_dict
        else:
            return None


def check_password(username, password):
    correct_password_hash = get_user_password_from_db(username=username)
    try:
        if correct_password_hash and (correct_password_hash == hashlib.md5(password.encode("utf-8")).hexdigest()):
            return True
        else:
            return False
    except Exception as e:
        return False


def add_warning_to_db(data):
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        query = '''
            INSERT INTO warnings(date, description)
            VALUES (?, ?)
        '''
        try:
            cursor.execute(query, (
                data['date'],
                data['description']
            ))
            conn.commit()
        except Exception as e:
            return -1


def get_all_warnings():
    with create_sqlite_connection() as conn:
        cursor = conn.cursor()
        try:
            # Select all columns from the warnings table
            cursor.execute('SELECT * FROM warnings ORDER BY id DESC')

            # Fetch all rows as a list of dictionaries
            warnings = []
            columns = [column[0] for column in cursor.description]
            for row in cursor.fetchall():
                warning_dict = dict(zip(columns, row))
                warnings.append(warning_dict)

            return warnings
        except sqlite3.Error as e:
            print(f"Error fetching warnings: {e}")
            return None


def warnings_seen():
    with create_sqlite_connection() as conn:

        # Create a cursor
        cursor = conn.cursor()
        try:
            # Update the status of all warnings to be seen (e.g., status = 1)
            cursor.execute('UPDATE warnings SET status = 1')

            # Commit the changes to the database
            conn.commit()
        except sqlite3.Error as e:
            print(f"Error marking warnings as seen: {e}")


def wrong_password_attempt_warning():
    warning_data = {
        'date': datetime.now().strftime("%c"),  # You can replace this with the desired date format
        'description': 'Someone tried to enter your house with wrong password!!',
    }
    # Call the function to add the warning to the database
    result = add_warning_to_db(warning_data)
    return result

