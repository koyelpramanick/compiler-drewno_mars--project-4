set breakpoint pending on
set confirm off
file ./dmc
break drewno_mars::Err::report
commands
	where
end
break drewno_mars::InternalError::InternalError
commands
	where
end

define p4
  set args p4_tests/$arg0.dm -n
  run
end
