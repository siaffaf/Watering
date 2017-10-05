CREATE TABLE "users"(username text, password text, template text, PLC_IP text);
CREATE TABLE logs(node text, date text, user text, code integer, message text);
CREATE INDEX log_idx on logs (node,date);
CREATE TABLE err_messages(PLC_IP, err_code text, err_message text);

