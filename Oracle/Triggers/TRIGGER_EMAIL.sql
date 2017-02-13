create or replace
trigger trigger_email
before insert on email
for each row
begin
select seq_email.nextval into :new.id from dual;
end;