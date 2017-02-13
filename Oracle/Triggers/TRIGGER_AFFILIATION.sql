create or replace
trigger trigger_affiliation
before insert on affiliation
for each row
begin
select seq_affiliation.nextval into :new.id from dual;
end;